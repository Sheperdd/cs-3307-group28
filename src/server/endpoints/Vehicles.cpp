#include "Vehicles.h"
#include "DTOSerialization.h"

//  All business logic goes through CustomerService — no direct db calls.
net::awaitable<http::response<http::string_body>>
VehiclesHandler::handle(const http::request<http::string_body> &req,
                        const std::vector<std::string> &path_parts,
                        ServiceContext &ctx,
                        net::thread_pool &pool)
{
    const unsigned ver = req.version();
    const bool ka = req.keep_alive();

    // /vehicles/{id}  —  GET, PATCH, DELETE
    if (path_parts.size() == 2)
    {
        VehicleId id = http_utils::parse_int(path_parts[1]);
        if (id < 0)
            co_return http_utils::make_error(http::status::bad_request,
                                             "Invalid Vehicle ID", ver, ka);

        switch (req.method())
        {
        case http::verb::get:
            co_return co_await getVehicle(id, ver, ka, ctx, pool);
        case http::verb::patch:
            co_return co_await updateVehicle(id, req, ver, ka, ctx, pool);
        case http::verb::delete_:
            co_return co_await deleteVehicle(id, ver, ka, ctx, pool);
        default:
            co_return http_utils::make_error(http::status::method_not_allowed,
                                             "Method not allowed", ver, ka);
        }
    }

    // /users/{userID}/vehicles  —  GET (list), POST (create)
    if (path_parts.size() == 3)
    {
        UserId userId = http_utils::parse_int(path_parts[1]);
        if (userId < 0)
            co_return http_utils::make_error(http::status::bad_request,
                                             "Invalid User ID", ver, ka);

        switch (req.method())
        {
        case http::verb::get:
            co_return co_await listVehiclesForUser(userId, ver, ka, ctx, pool);
        case http::verb::post:
            co_return co_await createVehicle(userId, req, ver, ka, ctx, pool);
        default:
            co_return http_utils::make_error(http::status::method_not_allowed,
                                             "Method not allowed", ver, ka);
        }
    }

    co_return http_utils::make_error(http::status::not_found,
                                     "Not found", ver, ka);
}

// GET /vehicles/{id}
net::awaitable<http::response<http::string_body>>
VehiclesHandler::getVehicle(VehicleId id, unsigned ver, bool ka,
                            ServiceContext &ctx, net::thread_pool &pool)
{
    // Service methods throw on error; we catch inside the lambda so the
    // co_await never sits inside a catch block (C++20 coroutine rule).
    struct Result {
        std::optional<VehicleDTO> dto;
        std::string error;
        bool badRequest{false};
    };

    auto res = co_await net::co_spawn(
        pool,
        [&ctx, id]() -> net::awaitable<Result>
        {
            Result r;
            try {
                r.dto = ctx.customerService.getVehicle(id);
            } catch (const std::invalid_argument &e) {
                r.error = e.what();
                r.badRequest = true;
            } catch (const std::runtime_error &e) {
                r.error = e.what();
            }
            co_return r;
        },
        net::use_awaitable);

    if (res.dto.has_value())
        co_return http_utils::make_json_response(http::status::ok,
                                                 json(*res.dto), ver, ka);
    if (res.badRequest)
        co_return http_utils::make_error(http::status::bad_request,
                                         res.error, ver, ka);
    co_return http_utils::make_error(http::status::not_found,
                                     res.error, ver, ka);
}

// PATCH /vehicles/{id}
net::awaitable<http::response<http::string_body>>
VehiclesHandler::updateVehicle(VehicleId id,
                               const http::request<http::string_body> &req,
                               unsigned ver, bool ka,
                               ServiceContext &ctx, net::thread_pool &pool)
{
    json body;
    bool parseOk = true;
    try { body = json::parse(req.body()); }
    catch (...) { parseOk = false; }
    if (!parseOk)
        co_return http_utils::make_error(http::status::bad_request,
                                         "Invalid JSON body", ver, ka);

    VehicleUpdate updates = body.get<VehicleUpdate>();

    bool ok = co_await net::co_spawn(
        pool,
        [&ctx, id, updates]() -> net::awaitable<bool>
        {
            co_return ctx.customerService.updateVehicle(id, updates);
        },
        net::use_awaitable);

    if (!ok)
        co_return http_utils::make_error(http::status::internal_server_error,
                                         "Failed to update vehicle", ver, ka);

    co_return http_utils::make_json_response(http::status::ok,
                                             json{{"message", "Vehicle updated"}}, ver, ka);
}

// DELETE /vehicles/{id}
net::awaitable<http::response<http::string_body>>
VehiclesHandler::deleteVehicle(VehicleId id, unsigned ver, bool ka,
                               ServiceContext &ctx, net::thread_pool &pool)
{
    bool ok = co_await net::co_spawn(
        pool,
        [&ctx, id]() -> net::awaitable<bool>
        {
            co_return ctx.customerService.removeVehicle(id);
        },
        net::use_awaitable);

    if (!ok)
        co_return http_utils::make_error(http::status::internal_server_error,
                                         "Failed to delete vehicle", ver, ka);

    co_return http_utils::make_json_response(http::status::ok,
                                             json{{"message", "Vehicle deleted"}}, ver, ka);
}

// GET /user/{userId}/vehicles
net::awaitable<http::response<http::string_body>>
VehiclesHandler::listVehiclesForUser(UserId userId, unsigned ver, bool ka,
                                     ServiceContext &ctx, net::thread_pool &pool)
{
    struct Result {
        std::vector<VehicleDTO> vehicles;
        std::string error;
        bool badRequest{false};
    };

    auto res = co_await net::co_spawn(
        pool,
        [&ctx, userId]() -> net::awaitable<Result>
        {
            Result r;
            try {
                r.vehicles = ctx.customerService.listVehicles(userId);
            } catch (const std::invalid_argument &e) {
                r.error = e.what();
                r.badRequest = true;
            }
            co_return r;
        },
        net::use_awaitable);

    if (res.badRequest)
        co_return http_utils::make_error(http::status::bad_request,
                                         res.error, ver, ka);

    json arr = json::array();
    for (const auto &v : res.vehicles)
        arr.push_back(json(v));

    co_return http_utils::make_json_response(http::status::ok,
                                             arr, ver, ka);
}

// POST /user/{userId}/vehicles
net::awaitable<http::response<http::string_body>>
VehiclesHandler::createVehicle(UserId userId,
                               const http::request<http::string_body> &req,
                               unsigned ver, bool ka,
                               ServiceContext &ctx, net::thread_pool &pool)
{
    json body;
    bool parseOk = true;
    try { body = json::parse(req.body()); }
    catch (...) { parseOk = false; }
    if (!parseOk)
        co_return http_utils::make_error(http::status::bad_request,
                                         "Invalid JSON body", ver, ka);

    VehicleCreate vehicle = body.get<VehicleCreate>();

    struct Result {
        VehicleId id{-1};
        std::string error;
        bool badRequest{false};
    };

    auto res = co_await net::co_spawn(
        pool,
        [&ctx, userId, vehicle]() -> net::awaitable<Result>
        {
            Result r;
            try {
                r.id = ctx.customerService.addVehicle(userId, vehicle);
            } catch (const std::invalid_argument &e) {
                r.error = e.what();
                r.badRequest = true;
            } catch (const std::runtime_error &e) {
                r.error = e.what();
            }
            co_return r;
        },
        net::use_awaitable);

    if (res.badRequest)
        co_return http_utils::make_error(http::status::bad_request,
                                         res.error, ver, ka);
    if (res.id < 0)
        co_return http_utils::make_error(http::status::internal_server_error,
                                         res.error, ver, ka);

    co_return http_utils::make_json_response(http::status::created,
                                             json{{"id", res.id}}, ver, ka);
}

#include "Vehicles.h"
#include "RecordsSerialization.h"

//  router like we did in HttpSession, but for the /vehicles endpoint
net::awaitable<http::response<http::string_body>>
VehiclesHandler::handle(const http::request<http::string_body> &req,
                        const std::vector<std::string> &path_parts,
                        DatabaseManager &db,
                        net::thread_pool &pool)
{
    const unsigned ver = req.version();
    const bool ka = req.keep_alive();

    // /vehicles/{id}  —  GET, PATCH, DELETE
    if (path_parts.size() == 2)
    {
        // get vehicle ID from URL and check it's valid
        VehicleId id = http_utils::parse_int(path_parts[1]);
        if (id < 0)
            co_return http_utils::make_error(http::status::bad_request,
                                             "Invalid Vehicle ID", ver, ka);

        switch (req.method())
        {
        case http::verb::get:
            co_return co_await getVehicle(id, ver, ka, db, pool);
        case http::verb::patch:
            co_return co_await updateVehicle(id, req, ver, ka, db, pool);
        case http::verb::delete_:
            co_return co_await deleteVehicle(id, ver, ka, db, pool);
        default:
            co_return http_utils::make_error(http::status::method_not_allowed,
                                             "Method not allowed", ver, ka);
        }
    }

    // /users/{userID}/vehicles  —  GET (list), POST (create)
    if (path_parts.size() == 3)
    {
        // get user ID from URL and check it's valid
        UserId userId = http_utils::parse_int(path_parts[1]);
        if (userId < 0)
            co_return http_utils::make_error(http::status::bad_request,
                                             "Invalid User ID", ver, ka);

        switch (req.method())
        {
        case http::verb::get:
            co_return co_await listVehiclesForUser(userId, ver, ka, db, pool);
        case http::verb::post:
            co_return co_await createVehicle(userId, req, ver, ka, db, pool);
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
                            DatabaseManager &db, net::thread_pool &pool)
{
    // Fetch the vehicle record (ID) from the database asynchronously using co_spawn to offload to the thread pool
    auto vehicleOpt = co_await net::co_spawn(
        pool,
        [&db, id]() -> net::awaitable<std::optional<VehicleRecord>>
        {
            co_return db.getVehicleById(id);
        },
        net::use_awaitable);

    // If the vehicle doesn't exist, return a 404 error
    if (!vehicleOpt.has_value())
        co_return http_utils::make_error(http::status::not_found,
                                         "Vehicle not found", ver, ka);

    // here, we convert the VehicleRecord to JSON using the to_json function defined in RecordsSerialization.h, which is called indirectly by nlohmann
    json vehicleInfo = *vehicleOpt;
    co_return http_utils::make_json_response(http::status::ok,
                                             vehicleInfo, ver, ka);
}

// PATCH /vehicles/{id}
net::awaitable<http::response<http::string_body>>
VehiclesHandler::updateVehicle(VehicleId id,
                               const http::request<http::string_body> &req,
                               unsigned ver, bool ka,
                               DatabaseManager &db, net::thread_pool &pool)
{
    json body;
    bool parseOk = true;
    try
    {
        body = json::parse(req.body());
    }
    catch (...)
    {
        parseOk = false;
    }
    if (!parseOk)
        co_return http_utils::make_error(http::status::bad_request,
                                         "Invalid JSON body", ver, ka);

    VehicleUpdate updates = body.get<VehicleUpdate>();

    bool ok = co_await net::co_spawn(
        pool,
        [&db, id, updates]() -> net::awaitable<bool>
        {
            co_return db.updateVehicle(id, updates);
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
                               DatabaseManager &db, net::thread_pool &pool)
{
    bool ok = co_await net::co_spawn(
        pool,
        [&db, id]() -> net::awaitable<bool>
        {
            co_return db.deleteVehicle(id);
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
                                     DatabaseManager &db, net::thread_pool &pool)
{
    auto vehicles = co_await net::co_spawn(
        pool,
        [&db, userId]() -> net::awaitable<std::vector<VehicleRecord>>
        {
            co_return db.listVehiclesForUser(userId);
        },
        net::use_awaitable);

    json arr = json::array();
    for (const auto &v : vehicles)
        arr.push_back(json(v));

    co_return http_utils::make_json_response(http::status::ok,
                                             arr, ver, ka);
}

// POST /user/{userId}/vehicles
net::awaitable<http::response<http::string_body>>
VehiclesHandler::createVehicle(UserId userId,
                               const http::request<http::string_body> &req,
                               unsigned ver, bool ka,
                               DatabaseManager &db, net::thread_pool &pool)
{
    json body;
    bool parseOk = true;
    try
    {
        body = json::parse(req.body());
    }
    catch (...)
    {
        parseOk = false;
    }
    if (!parseOk)
        co_return http_utils::make_error(http::status::bad_request,
                                         "Invalid JSON body", ver, ka);

    VehicleRecord vehicle = body.get<VehicleRecord>();
    vehicle.ownerUserId = userId; // override with the URL userId

    VehicleId newId = co_await net::co_spawn(
        pool,
        [&db, userId, vehicle]() -> net::awaitable<VehicleId>
        {
            co_return db.createVehicle(userId, vehicle);
        },
        net::use_awaitable);

    if (newId < 0)
        co_return http_utils::make_error(http::status::internal_server_error,
                                         "Failed to create vehicle", ver, ka);

    co_return http_utils::make_json_response(http::status::created,
                                             json{{"id", newId}}, ver, ka);
}

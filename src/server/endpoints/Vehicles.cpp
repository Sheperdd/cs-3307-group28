/**
 * @file Vehicles.cpp
 * @brief Vehicle endpoint implementation — get, update, delete, create
 *        symptom form.
 */
#include "Vehicles.h"
#include "DTOSerialization.h"

//  All business logic goes through CustomerService — no direct db calls.
net::awaitable<http::response<http::string_body>>
VehiclesHandler::handle(const http::request<http::string_body> &req,
                        const std::vector<std::string> &path_parts,
                        ServiceContext &ctx,
                        net::thread_pool &pool,
                        const std::optional<AuthInfo> &auth)
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

    // /vehicles/{vehicleId}/symptoms  —  POST (create symptom form)
    if (path_parts.size() == 3 && path_parts[2] == "symptoms")
    {
        VehicleId vehicleId = http_utils::parse_int(path_parts[1]);
        if (vehicleId < 0)
            co_return http_utils::make_error(http::status::bad_request,
                                             "Invalid Vehicle ID", ver, ka);
        if (req.method() == http::verb::post)
            co_return co_await createSymptomForm(vehicleId, req, ver, ka, ctx, pool);

        co_return http_utils::make_error(http::status::method_not_allowed,
                                         "Method not allowed", ver, ka);
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

// POST /vehicles/{vehicleId}/symptoms
net::awaitable<http::response<http::string_body>>
VehiclesHandler::createSymptomForm(VehicleId vehicleId,
                                   const http::request<http::string_body> &req,
                                   unsigned ver, bool ka,
                                   ServiceContext &ctx, net::thread_pool &pool)
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
  SymptomFormCreate formCreate;
  try
  {
    formCreate = body.get<SymptomFormCreate>();
  }
  catch (const std::exception &e)
  {
    co_return http_utils::make_error(http::status::bad_request,
                                     std::string("Bad symptom form payload: ") + e.what(),
                                     ver, ka);
  }

  struct Result
  {
    SymptomFormId id{-1};
    std::string error;
    bool badRequest{false};
  };

  auto res = co_await net::co_spawn(
      pool,
      [&ctx, formCreate]() -> net::awaitable<Result>
      {
        Result r;
        try
        {
          r.id = ctx.customerService.createSymptomForm(formCreate);
        }
        catch (const std::invalid_argument &e)
        {
          r.error = e.what();
          r.badRequest = true;
        }
        catch (const std::exception &e)
        {
          r.error = e.what();
        }
        co_return r;
      },
      net::use_awaitable);

  if (res.badRequest)
    co_return http_utils::make_error(http::status::bad_request,
                                     res.error, ver, ka);
  if (!res.error.empty())
    co_return http_utils::make_error(http::status::internal_server_error,
                                     res.error, ver, ka);
  co_return http_utils::make_json_response(http::status::ok,
                                           json{{"symptomFormId", res.id}}, ver, ka);
}

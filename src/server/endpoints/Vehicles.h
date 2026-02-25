#pragma once

#include "EndpointHandler.h"

/// @brief Handles requests to /vehicles, /vehicles/{id}, and related vehicle endpoints.
///        Delegates all business logic to CustomerService via the ServiceContext.
class VehiclesHandler : public EndpointHandler
{
public:
  /// @brief Main entry point for handling requests to /vehicles and subpaths.
  net::awaitable<http::response<http::string_body>>
  handle(const http::request<http::string_body> &req,
         const std::vector<std::string> &path_parts,
         ServiceContext &ctx,
         net::thread_pool &pool) override;

private:
  // GET /vehicles/{id}
  net::awaitable<http::response<http::string_body>>
  getVehicle(VehicleId id, unsigned ver, bool ka,
             ServiceContext &ctx, net::thread_pool &pool);

  // PATCH /vehicles/{id}
  net::awaitable<http::response<http::string_body>>
  updateVehicle(VehicleId id, const http::request<http::string_body> &req,
                unsigned ver, bool ka,
                ServiceContext &ctx, net::thread_pool &pool);

  // DELETE /vehicles/{id}
  net::awaitable<http::response<http::string_body>>
  deleteVehicle(VehicleId id, unsigned ver, bool ka,
                ServiceContext &ctx, net::thread_pool &pool);

  // GET /user/{userId}/vehicles
  net::awaitable<http::response<http::string_body>>
  listVehiclesForUser(UserId userId, unsigned ver, bool ka,
                      ServiceContext &ctx, net::thread_pool &pool);

  // POST /user/{userId}/vehicles
  net::awaitable<http::response<http::string_body>>
  createVehicle(UserId userId, const http::request<http::string_body> &req,
                unsigned ver, bool ka,
                ServiceContext &ctx, net::thread_pool &pool);
};

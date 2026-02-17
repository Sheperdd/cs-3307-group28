#pragma once

#include "EndpointHandler.h"

/// @brief Handles requests to /vehicles, /vehicles/{id}, and related vehicle endpoints.
class VehiclesHandler : public EndpointHandler
{
public:
  /// @brief Main entry point for handling requests to /vehicles and subpaths.
  /// @param req The incoming HTTP request.
  /// @param path_parts The parts of the URL path
  /// @param db database manager
  /// @param pool thread pool
  /// @return An HTTP response to be sent back to the client.
  net::awaitable<http::response<http::string_body>>
  handle(const http::request<http::string_body> &req,
         const std::vector<std::string> &path_parts,
         DatabaseManager &db,
         net::thread_pool &pool) override;

private:
  // GET /vehicles/{id}
  net::awaitable<http::response<http::string_body>>
  getVehicle(VehicleId id, unsigned ver, bool ka,
             DatabaseManager &db, net::thread_pool &pool);

  // PUT /vehicles/{id}
  net::awaitable<http::response<http::string_body>>
  updateVehicle(VehicleId id, const http::request<http::string_body> &req,
                unsigned ver, bool ka,
                DatabaseManager &db, net::thread_pool &pool);

  // DELETE /vehicles/{id}
  net::awaitable<http::response<http::string_body>>
  deleteVehicle(VehicleId id, unsigned ver, bool ka,
                DatabaseManager &db, net::thread_pool &pool);

  // GET /vehicles/owner/{userId}
  net::awaitable<http::response<http::string_body>>
  listVehiclesForUser(UserId userId, unsigned ver, bool ka,
                      DatabaseManager &db, net::thread_pool &pool);

  // POST /vehicles/owner/{userId}
  net::awaitable<http::response<http::string_body>>
  createVehicle(UserId userId, const http::request<http::string_body> &req,
                unsigned ver, bool ka,
                DatabaseManager &db, net::thread_pool &pool);
};

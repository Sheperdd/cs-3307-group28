#pragma once

#include "EndpointHandler.h"

/// @brief Handles requests to /users and /users/{id}, as well as /login.
class CustomersHandler : public EndpointHandler
{
public:
  net::awaitable<http::response<http::string_body>>
  handle(const http::request<http::string_body> &req,
         const std::vector<std::string> &path_parts,
         ServiceContext &ctx,
         net::thread_pool &pool) override;

private:
  // POST /auth/register
  net::awaitable<http::response<http::string_body>>
  registerUser(const http::request<http::string_body> &req,
               unsigned ver, bool ka,
               ServiceContext &ctx, net::thread_pool &pool);
  // POST /auth/login
  net::awaitable<http::response<http::string_body>>
  loginUser(const http::request<http::string_body> &req,
            unsigned ver, bool ka,
            ServiceContext &ctx, net::thread_pool &pool);

  // GET /users/{id}
  net::awaitable<http::response<http::string_body>>
  getUser(UserId userId, unsigned ver, bool ka,
          ServiceContext &ctx, net::thread_pool &pool);
  // PATCH /users/{id}
  net::awaitable<http::response<http::string_body>>
  updateUser(UserId userId, const http::request<http::string_body> &req,
             unsigned ver, bool ka,
             ServiceContext &ctx, net::thread_pool &pool);
  // DELETE /users/{id}
  net::awaitable<http::response<http::string_body>>
  deleteUser(UserId userId, unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);

  // PATCH /users/{id}/password
  net::awaitable<http::response<http::string_body>>
  updatePassword(UserId userId, const http::request<http::string_body> &req,
                 unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);

  // GET /users/{id}/vehicles
  net::awaitable<http::response<http::string_body>>
  getVehiclesForUser(UserId userId, unsigned ver, bool ka,
                     ServiceContext &ctx, net::thread_pool &pool);
  // POST /users/{id}/vehicles
  net::awaitable<http::response<http::string_body>>
  addVehicleForUser(UserId userId, const http::request<http::string_body> &req,
                    unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);

  // GET /users/{id}/symptoms
  net::awaitable<http::response<http::string_body>>
  getSymptomFormsForUser(UserId userId, unsigned ver, bool ka,
                         ServiceContext &ctx, net::thread_pool &pool);

  // GET /users/{id}/appointments
  net::awaitable<http::response<http::string_body>>
  getAppointmentsForUser(UserId userId, unsigned ver, bool ka,
                         ServiceContext &ctx, net::thread_pool &pool);

  // GET /users/{id}/reviews
  net::awaitable<http::response<http::string_body>>
  getReviewsByUser(UserId userId, unsigned ver, bool ka,
                   ServiceContext &ctx, net::thread_pool &pool);
};

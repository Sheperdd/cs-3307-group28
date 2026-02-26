#pragma once

#include "EndpointHandler.h"

/// @brief Handles requests to /appointments and /appointments/{id}.
class AppointmentsHandler : public EndpointHandler
{
public:
  net::awaitable<http::response<http::string_body>>
  handle(const http::request<http::string_body> &req,
         const std::vector<std::string> &path_parts,
         ServiceContext &ctx,
         net::thread_pool &pool) override;

private:
  // Helper methods for handling specific appointment-related operations
  // POST /appointments
  net::awaitable<http::response<http::string_body>>
  create(const http::request<http::string_body> &req,
         unsigned ver, bool ka,
         ServiceContext &ctx, net::thread_pool &pool);
  // GET /appointments/{id}
  net::awaitable<http::response<http::string_body>> getById(AppointmentId id, unsigned ver, bool ka,
                                                            ServiceContext &ctx, net::thread_pool &pool);
  // PATCH /appointments/{id}/status
  net::awaitable<http::response<http::string_body>> updateStatus(AppointmentId id, const http::request<http::string_body> &req,
                                                                 unsigned ver, bool ka,
                                                                 ServiceContext &ctx, net::thread_pool &pool);
  // POST /appointments/{id}/job
  net::awaitable<http::response<http::string_body>> startJob(AppointmentId id, const http::request<http::string_body> &req,
                                                             unsigned ver, bool ka,
                                                             ServiceContext &ctx, net::thread_pool &pool);
};

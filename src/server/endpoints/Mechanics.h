#pragma once

#include "EndpointHandler.h"

/// @brief Handles requests to /mechanics and /mechanics/{id}.
class MechanicsHandler : public EndpointHandler
{
public:
  net::awaitable<http::response<http::string_body>>
  handle(const http::request<http::string_body> &req,
         const std::vector<std::string> &path_parts,
         ServiceContext &ctx,
         net::thread_pool &pool,
         const std::optional<AuthInfo> &auth) override;

private:
  // GET /mechanics
  net::awaitable<http::response<http::string_body>>
  search(unsigned ver, bool ka, const http::request<http::string_body> &req, ServiceContext &ctx, net::thread_pool &pool);
  // GET /mechanics/{id}
  net::awaitable<http::response<http::string_body>>
  getProfile(MechanicId id, unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);
  // PATCH /mechanics/{id}
  net::awaitable<http::response<http::string_body>>
  updateProfile(MechanicId id, const http::request<http::string_body> &req, unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);
  // GET /mechanics/{id}/jobs
  net::awaitable<http::response<http::string_body>>
  listOpenJobs(MechanicId id, unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);
  // GET /mechanics/{id}/appointments
  net::awaitable<http::response<http::string_body>>
  listMechanicAppts(MechanicId id, unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);
  // GET /mechanics/{id}/reviews
  net::awaitable<http::response<http::string_body>>
  listMechanicReviews(MechanicId id, unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);
};

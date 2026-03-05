#pragma once

#include "EndpointHandler.h"

/// @brief Handles requests to /jobs and /jobs/{id}.
class JobsHandler : public EndpointHandler
{
public:
  net::awaitable<http::response<http::string_body>>
  handle(const http::request<http::string_body> &req,
         const std::vector<std::string> &path_parts,
         ServiceContext &ctx,
         net::thread_pool &pool) override;

private:
  // GET /jobs/{id}
  net::awaitable<http::response<http::string_body>>
  getJob(JobId id, unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);
  // PATCH /jobs/{id}/stage
  net::awaitable<http::response<http::string_body>>
  updateStage(JobId id, const http::request<http::string_body> &req,
              unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);
  // POST /jobs/{id}/complete
  net::awaitable<http::response<http::string_body>>
  completeJob(JobId id, const http::request<http::string_body> &req,
              unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);
  // GET /jobs/{id}/notes
  net::awaitable<http::response<http::string_body>>
  getJobNotes(JobId id, unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);
  // POST /jobs/{id}/notes
  net::awaitable<http::response<http::string_body>>
  addJobNote(JobId id, const http::request<http::string_body> &req,
             unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);
};

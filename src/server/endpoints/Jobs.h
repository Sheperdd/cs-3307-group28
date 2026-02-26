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
  //POST /appointments/{id}/job
  net::awaitable<http::response<http::string_body>>
  startJob(UserId userId, const http::request<http::string_body> &req,
           unsigned ver, bool ka,
           ServiceContext &ctx, net::thread_pool &pool);
  // GET /jobs/{id}
  net::awaitable<http::response<http::string_body>>
  getJob(JobId id, unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);
  // GET /mechanics/{id}/jobs
  net::awaitable<http::response<http::string_body>>
  listOpenJobs(MechanicId id, unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);
  // PUT /jobs/{id}/stage
  net::awaitable<http::response<http::string_body>>
  updateStage(JobId id, const http::request<http::string_body> &req,
                 unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);
  // POST /jobs/{id}/complete
  net::awaitable<http::response<http::string_body>>
  completeJob(JobId id, const http::request<http::string_body> &req,
              unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);
};

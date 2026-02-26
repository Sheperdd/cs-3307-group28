#include "Jobs.h"

net::awaitable<http::response<http::string_body>>
JobsHandler::handle(const http::request<http::string_body> &req,
                    const std::vector<std::string> &path_parts,
                    ServiceContext &ctx,
                    net::thread_pool &pool)
{
  const unsigned ver = req.version();
  const bool ka = req.keep_alive();
  // Use a switch to determine which specific job-related operation to perform based on the URL path and HTTP method
  switch (path_parts.size())
  {
  case 2:
    if (req.method() == http::verb::get)
    {
      // GET /jobs/{id}
      JobId id = http_utils::parse_int(path_parts[1]);
      if (id < 0)
        co_return http_utils::make_error(http::status::bad_request,
                                         "Invalid Job ID", ver, ka);
      co_return co_await getJob(id, ver, ka, ctx, pool);
    }
    co_return http_utils::make_error(http::status::method_not_allowed,
                                     "Method not allowed", ver, ka);
  case 3:
  {
    JobId id = http_utils::parse_int(path_parts[1]);
    if (id < 0)
      co_return http_utils::make_error(http::status::bad_request,
                                       "Invalid Job ID", ver, ka);
    if (path_parts[2] == "stage" && req.method() == http::verb::put)
    {
      // PUT /jobs/{id}/stage
      co_return co_await updateStage(id, req, ver, ka, ctx, pool);
    }
    if (path_parts[2] == "complete" && req.method() == http::verb::post)
    {
      // POST /jobs/{id}/complete
      co_return co_await completeJob(id, req, ver, ka, ctx, pool);
    }
    co_return http_utils::make_error(http::status::not_found,
                                     "Not found", ver, ka);
  }
  default:
    co_return http_utils::make_error(http::status::not_found,
                                     "Not found", ver, ka);
  }
}

// GET /jobs/{id}
net::awaitable<http::response<http::string_body>>
JobsHandler::getJob(JobId id, unsigned ver, bool ka,
                    ServiceContext &ctx, net::thread_pool &pool)
{
  // TODO: Refactor to use ctx.mechanicService.getJobById()
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", ver, ka);
}

// PUT /jobs/{id}/stage
net::awaitable<http::response<http::string_body>>
JobsHandler::updateStage(JobId id, const http::request<http::string_body> &req,
                         unsigned ver, bool ka,
                         ServiceContext &ctx, net::thread_pool &pool)
{
  // TODO: Refactor to use ctx.mechanicService.updateJobStage()
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", ver, ka);
}

// POST /jobs/{id}/complete
net::awaitable<http::response<http::string_body>>
JobsHandler::completeJob(JobId id, const http::request<http::string_body> &req,
                         unsigned ver, bool ka,
                         ServiceContext &ctx, net::thread_pool &pool)
{
  // TODO: Refactor to use ctx.mechanicService.markJobComplete()
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", ver, ka);
}

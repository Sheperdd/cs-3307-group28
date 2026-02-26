#include "Mechanics.h"

net::awaitable<http::response<http::string_body>>
MechanicsHandler::handle(const http::request<http::string_body> &req,
                         const std::vector<std::string> &path_parts,
                         ServiceContext &ctx,
                         net::thread_pool &pool)
{
  const unsigned ver = req.version();
  const bool ka = req.keep_alive();
  // Use a switch to determine which specific mechanic-related operation to perform based on the URL path and HTTP method
  switch (path_parts.size())
  {
  case 1:
    if (req.method() == http::verb::get)
    {
      // GET /mechanics
      co_return co_await search(ver, ka, req, ctx, pool);
    }
    co_return http_utils::make_error(http::status::method_not_allowed,
                                     "Method not allowed", ver, ka);
  case 2:
  {
    MechanicId id = http_utils::parse_int(path_parts[1]);
    if (id < 0)
      co_return http_utils::make_error(http::status::bad_request,
                                       "Invalid Mechanic ID", ver, ka);
    if (req.method() == http::verb::get)
    {
      // GET /mechanics/{id}
      co_return co_await getProfile(id, ver, ka, ctx, pool);
    }
    if (req.method() == http::verb::put)
    {
      // PUT /mechanics/{id}
      co_return co_await updateProfile(id, req, ver, ka, ctx, pool);
    }
    co_return http_utils::make_error(http::status::method_not_allowed,
                                     "Method not allowed", ver, ka);
  }
  case 3:
  {
    MechanicId id = http_utils::parse_int(path_parts[1]);
    if (id < 0)
      co_return http_utils::make_error(http::status::bad_request,
                                       "Invalid Mechanic ID", ver, ka);
    if (path_parts[2] == "schedule")
    {
      if (req.method() == http::verb::get)
      {
        // GET /mechanics/{id}/schedule
        co_return co_await getAvailability(id, ver, ka, req, ctx, pool);
      }
      if (req.method() == http::verb::put)
      {
        // PUT /mechanics/{id}/schedule
        co_return co_await setAvailability(id, req, ver, ka, ctx, pool);
      }
      co_return http_utils::make_error(http::status::method_not_allowed,
                                       "Method not allowed", ver, ka);
    }
    if (path_parts[2] == "jobs" && req.method() == http::verb::get)
    {
      // GET /mechanics/{id}/jobs
      co_return co_await listOpenJobs(id, ver, ka, ctx, pool);
    }
    if (path_parts[2] == "appointments" && req.method() == http::verb::get)
    {
      // GET /mechanics/{id}/appointments
      co_return co_await listMechanicAppts(id, ver, ka, ctx, pool);
    }
    if (path_parts[2] == "reviews" && req.method() == http::verb::get)
    {
      // GET /mechanics/{id}/reviews
      co_return co_await listMechanicReviews(id, ver, ka, ctx, pool);
    }
    co_return http_utils::make_error(http::status::not_found,
                                     "Not found", ver, ka);
  }
  default:
    co_return http_utils::make_error(http::status::not_found,
                                     "Not found", ver, ka);
  }
}

// GET /mechanics
net::awaitable<http::response<http::string_body>>
MechanicsHandler::search(unsigned ver, bool ka, const http::request<http::string_body> &req,
                         ServiceContext &ctx, net::thread_pool &pool)
{
  // TODO: Refactor to use ctx.mechanicService.searchMechanics()
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", ver, ka);
}

// GET /mechanics/{id}
net::awaitable<http::response<http::string_body>>
MechanicsHandler::getProfile(MechanicId id, unsigned ver, bool ka,
                             ServiceContext &ctx, net::thread_pool &pool)
{
  // TODO: Refactor to use ctx.mechanicService.getMechanicByUserId()
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", ver, ka);
}

// PUT /mechanics/{id}
net::awaitable<http::response<http::string_body>>
MechanicsHandler::updateProfile(MechanicId id, const http::request<http::string_body> &req,
                                unsigned ver, bool ka,
                                ServiceContext &ctx, net::thread_pool &pool)
{
  // TODO: Refactor to use ctx.mechanicService.updateMechanicProfile()
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", ver, ka);
}

// GET /mechanics/{id}/schedule
net::awaitable<http::response<http::string_body>>
MechanicsHandler::getAvailability(MechanicId id, unsigned ver, bool ka,
                                  const http::request<http::string_body> &req,
                                  ServiceContext &ctx, net::thread_pool &pool)
{
  // TODO: Refactor to use ctx.mechanicService.getMechanicAvailability()
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", ver, ka);
}

// PUT /mechanics/{id}/schedule
net::awaitable<http::response<http::string_body>>
MechanicsHandler::setAvailability(MechanicId id, const http::request<http::string_body> &req,
                                  unsigned ver, bool ka,
                                  ServiceContext &ctx, net::thread_pool &pool)
{
  // TODO: Refactor to use ctx.mechanicService.setMechanicAvailability()
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", ver, ka);
}

// GET /mechanics/{id}/jobs
net::awaitable<http::response<http::string_body>>
MechanicsHandler::listOpenJobs(MechanicId id, unsigned ver, bool ka,
                               ServiceContext &ctx, net::thread_pool &pool)
{
  // TODO: Refactor to use ctx.mechanicService.listOpenJobsForMechanic()
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", ver, ka);
}

// GET /mechanics/{id}/appointments
net::awaitable<http::response<http::string_body>>
MechanicsHandler::listMechanicAppts(MechanicId id, unsigned ver, bool ka,
                                    ServiceContext &ctx, net::thread_pool &pool)
{
  // TODO: Refactor to use ctx.mechanicService.listAppointmentsForMechanic()
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", ver, ka);
}

// GET /mechanics/{id}/reviews
net::awaitable<http::response<http::string_body>>
MechanicsHandler::listMechanicReviews(MechanicId id, unsigned ver, bool ka,
                                      ServiceContext &ctx, net::thread_pool &pool)
{
  // TODO: Refactor to use ctx.mechanicService.listReviewsForMechanic()
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", ver, ka);
}

/**
 * @file Mechanics.cpp
 * @brief Mechanic endpoint implementation — search, profile, jobs, appointments, reviews.
 */
#include "Mechanics.h"

net::awaitable<http::response<http::string_body>>
MechanicsHandler::handle(const http::request<http::string_body> &req,
                         const std::vector<std::string> &path_parts,
                         ServiceContext &ctx,
                         net::thread_pool &pool,
                         const std::optional<AuthInfo> &auth)
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

// GET /mechanics?specialty=<s>&maxDistanceKm=<d>
net::awaitable<http::response<http::string_body>>
MechanicsHandler::search(unsigned ver, bool ka, const http::request<http::string_body> &req,
                         ServiceContext &ctx, net::thread_pool &pool)
{
  // Parse optional query parameters into a MechanicSearchFilter.
  auto params = http_utils::parse_query_params(req.target());

  MechanicSearchFilter filter;
  if (auto it = params.find("specialty"); it != params.end())
    filter.specialty = it->second;
  bool parseOk = true;
  if (auto it = params.find("maxDistanceKm"); it != params.end())
  {
    try
    {
      filter.maxDistanceKm = std::stod(it->second);
    }
    catch (...)
    {
      parseOk = false;
    }

    if (!parseOk)
      co_return http_utils::make_error(http::status::bad_request,
                                       "Invalid maxDistanceKm value", ver, ka);
  }

  struct Result
  {
    std::vector<MechanicDTO> mechanics;
    std::string error;
  };

  auto res = co_await net::co_spawn(
      pool,
      [&ctx, filter]() -> net::awaitable<Result>
      {
        Result r;
        try
        {
          r.mechanics = ctx.mechanicService.searchMechanics(filter);
        }
        catch (const std::exception &e)
        {
          r.error = e.what();
        }
        co_return r;
      },
      net::use_awaitable);

  if (!res.error.empty())
    co_return http_utils::make_error(http::status::internal_server_error,
                                     res.error, ver, ka);

  co_return http_utils::make_json_response(http::status::ok,
                                           json(res.mechanics), ver, ka);
}

// GET /mechanics/{id}
net::awaitable<http::response<http::string_body>>
MechanicsHandler::getProfile(MechanicId mechanicId, unsigned ver, bool ka,
                             ServiceContext &ctx, net::thread_pool &pool)
{
  struct Result
  {
    std::optional<MechanicDTO> mechanic;
    std::string error;
  };

  auto res = co_await net::co_spawn(
      pool,
      [&ctx, mechanicId]() -> net::awaitable<Result>
      {
        Result r;
        try
        {
          r.mechanic = ctx.mechanicService.getMechanicProfile(mechanicId);
        }
        catch (const std::exception &e)
        {
          r.error = e.what();
        }
        co_return r;
      },
      net::use_awaitable);

  if (!res.error.empty())
    co_return http_utils::make_error(http::status::internal_server_error,
                                     res.error, ver, ka);

  if (!res.mechanic.has_value())
    co_return http_utils::make_error(http::status::not_found,
                                     "Mechanic not found", ver, ka);

  co_return http_utils::make_json_response(http::status::ok,
                                           json(res.mechanic.value()), ver, ka);
}

// PATCH /mechanics/{id}
net::awaitable<http::response<http::string_body>>
MechanicsHandler::updateProfile(MechanicId id, const http::request<http::string_body> &req,
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

  MechanicUpdateDTO updateMechanic = body.get<MechanicUpdateDTO>();

  struct Result
  {
    bool ok{false};
    std::string error;
  };

  auto res = co_await net::co_spawn(
      pool,
      [&ctx, id, updateMechanic]() -> net::awaitable<Result>
      {
        Result r;
        try
        {
          r.ok = ctx.mechanicService.updateMechanicProfile(id, updateMechanic);
        }
        catch (const std::exception &e)
        {
          r.error = e.what();
        }
        co_return r;
      },
      net::use_awaitable);

  if (!res.error.empty())
    co_return http_utils::make_error(http::status::internal_server_error,
                                     res.error, ver, ka);

  co_return http_utils::make_json_response(http::status::ok,
                                           json{{"message", "Mechanic profile updated"}}, ver, ka);
}

// GET /mechanics/{id}/jobs
net::awaitable<http::response<http::string_body>>
MechanicsHandler::listOpenJobs(MechanicId mechId, unsigned ver, bool ka,
                               ServiceContext &ctx, net::thread_pool &pool)
{
  struct Result
  {
    std::vector<JobDTO> jobs;
    std::string error;
  };

  auto res = co_await net::co_spawn(
      pool,
      [&ctx, mechId]() -> net::awaitable<Result>
      {
        Result r;
        try
        {
          r.jobs = ctx.mechanicService.listOpenJobs(mechId);
        }
        catch (const std::exception &e)
        {
          r.error = e.what();
        }
        co_return r;
      },
      net::use_awaitable);

  if (!res.error.empty())
    co_return http_utils::make_error(http::status::internal_server_error,
                                     res.error, ver, ka);

  co_return http_utils::make_json_response(http::status::ok,
                                           json(res.jobs), ver, ka);
}

// GET /mechanics/{id}/appointments
net::awaitable<http::response<http::string_body>>
MechanicsHandler::listMechanicAppts(MechanicId id, unsigned ver, bool ka,
                                    ServiceContext &ctx, net::thread_pool &pool)
{
  struct Result
  {
    std::vector<AppointmentDTO> appointments;
    std::string error;
  };

  auto res = co_await net::co_spawn(
      pool,
      [&ctx, id]() -> net::awaitable<Result>
      {
        Result r;
        try
        {
          r.appointments = ctx.mechanicService.listIncomingRequests(id);
        }
        catch (const std::exception &e)
        {
          r.error = e.what();
        }
        co_return r;
      },
      net::use_awaitable);

  if (!res.error.empty())
    co_return http_utils::make_error(http::status::internal_server_error,
                                     res.error, ver, ka);

  co_return http_utils::make_json_response(http::status::ok,
                                           json(res.appointments), ver, ka);
}

// GET /mechanics/{id}/reviews
net::awaitable<http::response<http::string_body>>
MechanicsHandler::listMechanicReviews(MechanicId mechId, unsigned ver, bool ka,
                                      ServiceContext &ctx, net::thread_pool &pool)
{
  struct Result
  {
    std::vector<ReviewDTO> reviews;
    std::string error;
  };

  auto res = co_await net::co_spawn(
      pool,
      [&ctx, mechId]() -> net::awaitable<Result>
      {
        Result r;
        try
        {
          r.reviews = ctx.mechanicService.listMyReviews(mechId);
        }
        catch (const std::exception &e)
        {
          r.error = e.what();
        }
        co_return r;
      },
      net::use_awaitable);

  if (!res.error.empty())
    co_return http_utils::make_error(http::status::internal_server_error,
                                     res.error, ver, ka);

  co_return http_utils::make_json_response(http::status::ok,
                                           json(res.reviews), ver, ka);
}

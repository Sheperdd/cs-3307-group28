#include "Appointments.h"

using json = nlohmann::json;

net::awaitable<http::response<http::string_body>>
AppointmentsHandler::handle(const http::request<http::string_body> &req,
                            const std::vector<std::string> &path_parts,
                            ServiceContext &ctx,
                            net::thread_pool &pool)
{

  const unsigned ver = req.version();
  const bool ka = req.keep_alive();
  // Use a switch to determine which specific appointment-related operation to perform based on the URL path and HTTP method
  switch (path_parts.size())
  {
  case 1:
    if (req.method() == http::verb::post)
    {
      // POST /appointments
      co_return co_await createAppointment(req, ver, ka, ctx, pool);
    }
    co_return http_utils::make_error(http::status::method_not_allowed,
                                     "Method not allowed", ver, ka);
  case 2:
    if (req.method() == http::verb::get)
    {
      // GET /appointments/{id}
      AppointmentId id = http_utils::parse_int(path_parts[1]);
      if (id < 0)
        co_return http_utils::make_error(http::status::bad_request,
                                         "Invalid Appointment ID", ver, ka);
      co_return co_await getAppointmentById(id, ver, ka, ctx, pool);
    }
    co_return http_utils::make_error(http::status::method_not_allowed,
                                     "Method not allowed", ver, ka);
  case 3:
  {
    AppointmentId id = http_utils::parse_int(path_parts[1]);
    if (id < 0)
      co_return http_utils::make_error(http::status::bad_request,
                                       "Invalid Appointment ID", ver, ka);
    if (path_parts[2] == "status" && req.method() == http::verb::patch)
    {
      // PATCH /appointments/{id}/status
      co_return co_await updateAppointmentStatus(id, req, ver, ka, ctx, pool);
    }
    if (path_parts[2] == "job" && req.method() == http::verb::post)
    {
      // POST /appointments/{id}/job
      co_return co_await startJob(id, req, ver, ka, ctx, pool);
    }
    co_return http_utils::make_error(http::status::not_found,
                                     "Not found", ver, ka);
  }
  default:
    co_return http_utils::make_error(http::status::not_found,
                                     "Not found", ver, ka);
  }
}

// POST /appointments
net::awaitable<http::response<http::string_body>>
AppointmentsHandler::createAppointment(const http::request<http::string_body> &req,
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

  // TODO implement actually calling the function
  AppointmentCreate appointment;
  try
  {
    appointment = body.get<AppointmentCreate>();
  }
  catch (const std::exception &e)
  {
    co_return http_utils::make_error(http::status::bad_request,
                                     std::string("Bad appointment payload: ") + e.what(),
                                     ver, ka);
  }
  struct Result
  {
    AppointmentId id{-1};
    std::string error;
    bool badRequest{false};
  };

  auto res = co_await net::co_spawn(
      pool,
      [&ctx, appointment]() -> net::awaitable<Result>
      {
        Result r;
        try
        {
          r.id = ctx.customerService.requestAppointment(appointment);
        }
        catch (const std::invalid_argument &e)
        {
          r.error = e.what();
          r.badRequest = true;
        }
        catch (const std::exception &e)
        {
          r.error = e.what();
        }
        co_return r;
      },
      net::use_awaitable);

  if (res.badRequest)
    co_return http_utils::make_error(http::status::bad_request,
                                     res.error, ver, ka);
  if (!res.error.empty())
    co_return http_utils::make_error(http::status::internal_server_error,
                                     res.error, ver, ka);

  co_return http_utils::make_json_response(http::status::created,
                                           json{{"appointmentId", res.id}}, ver, ka);
}

// GET /appointments/{id}
net::awaitable<http::response<http::string_body>>
AppointmentsHandler::getAppointmentById(AppointmentId id, unsigned ver, bool ka,
                                        ServiceContext &ctx, net::thread_pool &pool)
{
  struct Result
  {
    std::optional<AppointmentDTO> appointment;
    std::string error;
  };

  auto res = co_await net::co_spawn(
      pool,
      [&ctx, id]() -> net::awaitable<Result>
      {
        Result r;
        try
        {
          r.appointment = ctx.customerService.getAppointment(id);
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
  if (!res.appointment.has_value())
    co_return http_utils::make_error(http::status::not_found,
                                     "Appointment not found", ver, ka);

  co_return http_utils::make_json_response(http::status::ok,
                                           json(*res.appointment), ver, ka);
}

// PATCH /appointments/{id}/status
net::awaitable<http::response<http::string_body>>
AppointmentsHandler::updateAppointmentStatus(AppointmentId id, const http::request<http::string_body> &req,
                                             unsigned ver, bool ka,
                                             ServiceContext &ctx, net::thread_pool &pool)
{
  // change the status of the appointment. The new status will be passed in the body as JSON, along with an optional note (e.g. reason for cancellation)
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

  AppointmentDTO appointment;
  AppointmentStatus statusUpdate;
  try
  {
    appointment = body.get<AppointmentDTO>();
    statusUpdate = appointment.status;
  }
  catch (const std::exception &e)
  {
    co_return http_utils::make_error(http::status::bad_request,
                                     std::string("Bad status update payload: ") + e.what(), ver, ka);
  }

  struct Result
  {
    bool success{false};
    std::string error;
  };

  auto res = co_await net::co_spawn(
      pool,
      [&ctx, id, statusUpdate]() -> net::awaitable<Result>
      {
        Result r;
        try
        {
          ctx.customerService.updateAppointmentStatus(id, statusUpdate);
          r.success = true;
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
  if (!res.success)    co_return http_utils::make_error(http::status::bad_request,
                                     "Could not update appointment status", ver, ka);
  co_return http_utils::make_json_response(http::status::ok,
                                           json{{"message", "Appointment status updated successfully"}}, ver, ka);
}

// POST /appointments/{id}/job
// TODO: FOR THIS ENDPOINT, WE PROBABLY ALSO NEED TO PASS THE MECHANIC ID IN THE BODY, OR SOMEHOW AUTHENTICATE THE MECHANIC, TO ENSURE THEY HAVE THE RIGHT TO START A JOB FOR THIS APPOINTMENT
// FOR NOW WE PASS THE ENTIRE APPOINTMENT DTO
net::awaitable<http::response<http::string_body>>
AppointmentsHandler::startJob(AppointmentId id, const http::request<http::string_body> &req,
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
  AppointmentDTO appointment;
  try
  {
    appointment = body.get<AppointmentDTO>();
  }
  catch (const std::exception &e)
  {
    co_return http_utils::make_error(http::status::bad_request,
                                     std::string("Bad appointment payload: ") + e.what(),
                                     ver, ka);
  }

  struct Result
  {
    bool success{false};
    std::string error;
  };

  auto res = co_await net::co_spawn(
      pool,
      [&ctx, id, appointment]() mutable -> net::awaitable<Result>
      {
        Result r;
        try
        {
          ctx.mechanicService.startJobFromAppointment(appointment.mechanicId, id);
          r.success = true;
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
  if (!res.success)
    co_return http_utils::make_error(http::status::bad_request,
                                     "Could not start job from appointment", ver, ka);

  co_return http_utils::make_json_response(http::status::ok,
                                           json{{"message", "Job started successfully"}}, ver, ka);
}
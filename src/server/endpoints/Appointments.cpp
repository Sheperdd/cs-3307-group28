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

  
  // TODO: Refactor to use ctx.customerService.requestAppointment()
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", ver, ka);
}

// GET /appointments/{id}
net::awaitable<http::response<http::string_body>>
AppointmentsHandler::getAppointmentById(AppointmentId id, unsigned ver, bool ka,
                                        ServiceContext &ctx, net::thread_pool &pool)
{
  // TODO: Refactor to use ctx.customerService.getAppointment() or ctx.mechanicService.getAppointmentDetails()
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", ver, ka);
}

// PATCH /appointments/{id}/status
net::awaitable<http::response<http::string_body>>
AppointmentsHandler::updateAppointmentStatus(AppointmentId id, const http::request<http::string_body> &req,
                                  unsigned ver, bool ka,
                                  ServiceContext &ctx, net::thread_pool &pool)
{
  // TODO: Refactor to use ctx.mechanicService.AcceptAppointment() / declineAppointment()
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", ver, ka);
}

// POST /appointments/{id}/job
net::awaitable<http::response<http::string_body>>
AppointmentsHandler::startJob(AppointmentId id, const http::request<http::string_body> &req,
                              unsigned ver, bool ka,
                              ServiceContext &ctx, net::thread_pool &pool)
{
  // TODO: Refactor to use ctx.mechanicService.createJobFromAppointment()
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", ver, ka);
}
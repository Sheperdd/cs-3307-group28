#include "Appointments.h"

using json = nlohmann::json;

net::awaitable<http::response<http::string_body>>
AppointmentsHandler::handle(const http::request<http::string_body> &req,
                            const std::vector<std::string> &path_parts,
                            DatabaseManager &db,
                            net::thread_pool &pool)
{

  const unsigned ver = req.version();
  const bool ka = req.keep_alive();
  // Use a switch to determine which specific appointment-related operation to perform based on the URL path and HTTP method
  switch (path_parts.size())
  {
  case 2:
    if(req.method() == http::verb::get)
    {
      // GET /appointments/{id}
      AppointmentId id = http_utils::parse_int(path_parts[1]);
      if (id < 0)
        co_return http_utils::make_error(http::status::bad_request,
                                         "Invalid Appointment ID", ver, ka);
      co_return co_await getById(id, ver, ka, db, pool);
    }
    else if(req.method() == http::verb::patch)
    {
      // PATCH /appointments/{id}/status
      AppointmentId id = http_utils::parse_int(path_parts[1]);
      if (id < 0)
        co_return http_utils::make_error(http::status::bad_request,
                                         "Invalid Appointment ID", ver, ka);
      co_return co_await updateStatus(id, req, ver, ka, db, pool);
    }
  case 3:
    co_return http_utils::make_error(http::status::not_implemented,
                                     "Not implemented", ver, ka);
  default:
    co_return http_utils::make_error(http::status::not_found,
                                     "Not found", ver, ka);
  }
}

// POST /appointments
net::awaitable<http::response<http::string_body>>
AppointmentsHandler::create(UserId userId, const http::request<http::string_body> &req,
                            unsigned ver, bool ka,
                            DatabaseManager &db, net::thread_pool &pool)
{
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", ver, ka);
}

// GET /appointments/{id}
net::awaitable<http::response<http::string_body>>
AppointmentsHandler::getById(AppointmentId id, unsigned ver, bool ka,
                             DatabaseManager &db, net::thread_pool &pool)
{
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", ver, ka);
}

// GET /users/{userId}/appointments
net::awaitable<http::response<http::string_body>>
AppointmentsHandler::listCustomerAppts(UserId userId, unsigned ver, bool ka,
                                       DatabaseManager &db, net::thread_pool &pool)
{
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", ver, ka);
}

// GET /mechanics/{mechanicId}/appointments
net::awaitable<http::response<http::string_body>>
AppointmentsHandler::listMechanicAppts(UserId mechanicId, unsigned ver, bool ka,
                                       DatabaseManager &db, net::thread_pool &pool)
{
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", ver, ka);
}

// PATCH /appointments/{id}/status
net::awaitable<http::response<http::string_body>>
AppointmentsHandler::updateStatus(AppointmentId id, const http::request<http::string_body> &req,
                                  unsigned ver, bool ka,
                                  DatabaseManager &db, net::thread_pool &pool)
{
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", ver, ka);
}
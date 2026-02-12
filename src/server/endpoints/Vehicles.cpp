#include "Vehicles.h"

net::awaitable<http::response<http::string_body>>
VehiclesHandler::handle(const http::request<http::string_body> &req,
                        const std::vector<std::string> &path_parts,
                        DatabaseManager &db,
                        net::thread_pool &pool)
{
  // TODO: Implement vehicle CRUD logic
  if (path_parts.size() == 2)
  {
    // GET /vehicles/{id}
    if (req.method() == http::verb::get)
    {
      // get vehicle id and make sure its valid
      int vehicleId = http_utils::parse_int(path_parts[1]);
      if (vehicleId < 0) // TODO: make sure that database starts iding at 0 and not 1
      {
        // Using coroutines to return an error response
        co_return http_utils::make_error(http::status::bad_request,
                                         "Invalid Vehicle ID", req.version(), req.keep_alive());
      }

      // get vehicle info from database
      json vehicleInfo = db.

                         if (vehicleInfo.contains("error")) // TODO: make sure that when theres an error in getting the info from db, that it will contain error. Could be something like NULL. Talk with aaron when theyre done
      {
        co_return http_utils::make_error(http::status::not_found,
                                         vehicleInfo["error"], req.version(), req.keep_alive());
      }

      co_return http_utils::make_json_response(http::status::ok,
                                               vehicleInfo, req.version(), req.keep_alive());
    }

    // PUT /vehicles/{id}
    if (req.method() == http::verb::put)
    {
      json body;
      try
      {
        body = json::parse(req.body());
      }
      catch (...)
      {
        co_return http_utils::make_error(http::status::bad_request,
                                         "Invalid JSON body", req.version(), req.keep_alive());
      }
    }

    // DELETE /vehicles/{id}
  }

  if (path_parts.size() == 3)
  {
    // GET /user/{userId}/vehicles

    // POST /user/{userId}/vehicles
  }
}

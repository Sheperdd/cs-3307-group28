#include "Appointments.h"

using json = nlohmann::json;

net::awaitable<http::response<http::string_body>>
AppointmentsHandler::handle(const http::request<http::string_body> &req,
                            const std::vector<std::string> &path_parts,
                            DatabaseManager &db,
                            net::thread_pool &pool)
{
  // POST /appointments
  // Check the request method and path (path_parts[0] should be "appointments")
  if (req.method() == http::verb::post && path_parts.size() == 1)
  {
    const unsigned ver = req.version();
    const bool ka = req.keep_alive();
    //
    if (req.method() != http::verb::post)
      co_return http_utils::make_error(http::status::method_not_allowed,
                                       "Method not allowed", ver, ka);

    // Get the body and convert it into json
    json body;
    try
    {
      body = json::parse(req.body());
    }
    catch (...)
    {
      co_return http_utils::make_error(http::status::bad_request, "Invalid JSON body", ver, ka);
    }

    // Validate and extract required fields
    try {
      if (!body.contains("customerId") || !body.contains("mechanicId") || 
          !body.contains("formId") || !body.contains("scheduledAt")) {
        co_return http_utils::make_error(http::status::bad_request, 
                                         "Missing required fields", ver, ka);
      }

      // Construct AppointmentRecord from JSON
      AppointmentRecord appointment;
      appointment.customerId = body["customerId"].get<UserId>();
      appointment.mechanicId = body["mechanicId"].get<MechanicId>();
      appointment.formId = body["formId"].get<SymptomFormId>();
      appointment.scheduledAt = body["scheduledAt"].get<std::string>();
      
      // Optional fields
      if (body.contains("note")) {
        appointment.note = body["note"].get<std::string>();
      }
      if (body.contains("status")) {
        // You'll need to convert string to AppointmentStatus enum
        appointment.status = AppointmentStatus::REQUESTED; // default
      } else {
        appointment.status = AppointmentStatus::REQUESTED; // default
      }

      // Call database to create appointment
      // auto appointmentId = co_await db.createAppointment(appointment);
      
      // For now, return success
      co_return http_utils::make_json_response(http::status::ok, 
                                               {{"message", "Appointment created"}}, 
                                               ver, ka);
    }
    catch (const json::exception& e) {
      co_return http_utils::make_error(http::status::bad_request, 
                                       std::string("Invalid field format: ") + e.what(), 
                                       ver, ka);
    }
  }
  // TODO: Implement appointment CRUD logic
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", req.version(), req.keep_alive());
}
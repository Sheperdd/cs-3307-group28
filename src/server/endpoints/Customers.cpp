#include "Customers.h"
#include "RecordsSerialization.h"

using json = nlohmann::json;

net::awaitable<http::response<http::string_body>>
CustomersHandler::handle(const http::request<http::string_body> &req,
                     const std::vector<std::string> &path_parts,
                     DatabaseManager &db,
                     net::thread_pool &pool)
{
    // GET /users/{id}
    // check the request method and path (path_parts[0] should be "users")
    if (req.method() == http::verb::get && path_parts.size() == 2)
    {
        // Get the user ID from the path and validate it
        int userId = http_utils::parse_int(path_parts[1]);
        if (userId < 0)
        {
            // Using coroutines to return an error response
            co_return http_utils::make_error(http::status::bad_request,
                                            "Invalid user ID", req.version(), req.keep_alive());
        }

        // Query the database for the user record
        auto userOpt = db.getUserRecordById(static_cast<UserId>(userId));

        if (!userOpt.has_value())
        {
            co_return http_utils::make_error(http::status::not_found,
                                            "User not found", req.version(), req.keep_alive());
        }

        // Return the user information as a JSON response
        json result = *userOpt;
        co_return http_utils::make_json_response(http::status::ok,
                                                result, req.version(), req.keep_alive());
    }

    // TODO: Implement other user endpoints
    co_return http_utils::make_error(http::status::not_implemented,
                                     "Not implemented", req.version(), req.keep_alive());
}
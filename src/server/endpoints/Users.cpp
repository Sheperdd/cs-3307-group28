#include "Users.h"

#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <charconv>

using json = nlohmann::json;

net::awaitable<http::response<http::string_body>>
UsersHandler::handle(const http::request<http::string_body> &req,
                     const std::vector<std::string> &path_parts,
                     DatabaseManager &db,
                     net::thread_pool &pool)
{
    // GET /users/{id}
    // check the request method and path (path_parts[0] should be "users")
    if (req.method() == http::verb::get && path_parts.size() == 2)
    {
        // Ge the user ID from the path and validate it
        int userId = http_utils::parse_int(path_parts[1]);
        if (userId < 0)
        {
            // Using coroutines to return an error response
            co_return http_utils::make_error(http::status::bad_request,
                                            "Invalid user ID", req.version(), req.keep_alive());
        }

        // Query the database for the user information
        json result = db.getUserById(userId);
        
        // Check if the result contains an error
        if (result.contains("error"))
        {
            // Return a not found error if the user is not found
            co_return http_utils::make_error(http::status::not_found,
                                            result["error"], req.version(), req.keep_alive());
        }

        // Return the user information as a JSON response
        co_return http_utils::make_json_response(http::status::ok,
                                                result, req.version(), req.keep_alive());
    }

    // TODO: Implement other user endpoints
    co_return http_utils::make_error(http::status::not_implemented,
                                     "Not implemented", req.version(), req.keep_alive());
}
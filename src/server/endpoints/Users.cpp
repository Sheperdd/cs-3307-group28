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
    if (req.method() == http::verb::get && path_parts.size() == 2)
    {
        int userId = http_utils::parse_int(path_parts[1]);
        if (userId < 0)
        {
            co_return http_utils::make_error(http::status::bad_request,
                                            "Invalid user ID", req.version(), req.keep_alive());
        }

        json result = db.getUserById(userId);
        
        // Check if the result contains an error
        if (result.contains("error"))
        {
            co_return http_utils::make_error(http::status::not_found,
                                            result["error"], req.version(), req.keep_alive());
        }

        co_return http_utils::make_json_response(http::status::ok,
                                                result, req.version(), req.keep_alive());
    }

    // TODO: Implement other user endpoints
    co_return http_utils::make_error(http::status::not_implemented,
                                     "Not implemented", req.version(), req.keep_alive());
}
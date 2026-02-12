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
    // TODO: Implement user CRUD and login logic
    co_return http_utils::make_error(http::status::not_implemented,
                                     "Not implemented", req.version(), req.keep_alive());
}

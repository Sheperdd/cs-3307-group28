#include "HttpSession.h"

#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <charconv>

using json = nlohmann::json;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

/// Splits a URI path like "/users/42" into segments: ["users", "42"]
static std::vector<std::string> split_path(std::string_view target)
{
    // Strip query string if present
    auto qpos = target.find('?');
    if (qpos != std::string_view::npos)
        target = target.substr(0, qpos);

    std::vector<std::string> parts;
    std::string_view::size_type start = 0;
    while (start < target.size())
    {
        if (target[start] == '/')
        {
            ++start;
            continue;
        }
        auto end = target.find('/', start);
        if (end == std::string_view::npos)
            end = target.size();
        parts.emplace_back(target.substr(start, end - start));
        start = end + 1;
    }
    return parts;
}

/// Try to parse a string_view as an int.  Returns -1 on failure.
static int parse_int(const std::string &s)
{
    int val = -1;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), val);
    if (ec != std::errc{} || ptr != s.data() + s.size())
        return -1;
    return val;
}

/// Build a JSON HTTP response with the given status code and body.
static http::response<http::string_body>
make_json_response(http::status status,
                   const json &body,
                   unsigned version,
                   bool keep_alive)
{
    http::response<http::string_body> res{status, version};
    res.set(http::field::content_type, "application/json");
    res.keep_alive(keep_alive);
    res.body() = body.dump();
    res.prepare_payload();
    return res;
}

/// Convenience overload for error responses.
static http::response<http::string_body>
make_error(http::status status,
           const std::string &message,
           unsigned version,
           bool keep_alive)
{
    return make_json_response(status, json{{"error", message}}, version, keep_alive);
}

// ---------------------------------------------------------------------------
// Top-level request router
// ---------------------------------------------------------------------------

static net::awaitable<http::response<http::string_body>>
route_request(const http::request<http::string_body> &req,
              DatabaseManager &db,
              net::thread_pool &pool)
{
    auto parts = split_path(req.target());

    if (parts.empty())
        co_return make_error(http::status::not_found,
                             "Not found", req.version(), req.keep_alive());

    if (parts[0] == "users")
        // co_return co_await handle_users(req, parts, db, pool);

        if (parts[0] == "login")
            // co_return co_await handle_login(req, db, pool);

            co_return make_error(http::status::not_found,
                                 "Not found", req.version(), req.keep_alive());
}

// ---------------------------------------------------------------------------
// The HTTP session coroutine  –  one per accepted connection
// ---------------------------------------------------------------------------

net::awaitable<void> http_session(tcp::socket socket,
                                  DatabaseManager &db,
                                  net::thread_pool &pool)
{
    beast::flat_buffer buffer;

    try
    {
        for (;;)
        {
            // Read one full HTTP request
            http::request<http::string_body> req;
            co_await http::async_read(socket, buffer, req, net::use_awaitable);

            std::cout << "[HTTP] " << req.method_string() << " " << req.target() << std::endl;

            // Route and build a response
            auto res = co_await route_request(req, db, pool);

            // Send the response
            co_await http::async_write(socket, res, net::use_awaitable);

            // If the client doesn't want keep-alive, close gracefully
            if (!res.keep_alive())
            {
                beast::error_code ec;
                socket.shutdown(tcp::socket::shutdown_send, ec);
                break;
            }
        }
    }
    catch (boost::system::system_error &se)
    {
        // end_of_stream means the client closed the connection – perfectly normal
        if (se.code() != http::error::end_of_stream)
            std::cerr << "Session error: " << se.what() << std::endl;
    }
    catch (std::exception &e)
    {
        std::cerr << "Session error: " << e.what() << std::endl;
    }
}

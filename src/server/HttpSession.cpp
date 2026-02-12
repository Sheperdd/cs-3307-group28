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
// Route: /users  and  /users/{id}
// ---------------------------------------------------------------------------

/// Dispatches a request that targets /users or /users/{id}.
static net::awaitable<http::response<http::string_body>>
handle_users(const http::request<http::string_body> &req,
             const std::vector<std::string> &parts,
             DatabaseManager &db,
             net::thread_pool &pool)
{
    const auto method = req.method();
    const unsigned ver = req.version();
    const bool ka = req.keep_alive();

    // ---- GET /users ----
    if (parts.size() == 1 && method == http::verb::get)
    {
        json users = co_await net::co_spawn(
            pool,
            [&db]() -> net::awaitable<json>
            {
                co_return db.getAllUsers();
            },
            net::use_awaitable);

        co_return make_json_response(http::status::ok, users, ver, ka);
    }

    // ---- POST /users ----
    if (parts.size() == 1 && method == http::verb::post)
    {
        json body;
        try
        {
            body = json::parse(req.body());
        }
        catch (...)
        {
            co_return make_error(http::status::bad_request,
                                 "Invalid JSON body", ver, ka);
        }

        if (!body.contains("name") || !body.contains("email") || !body.contains("password"))
        {
            co_return make_error(http::status::bad_request,
                                 "Missing required fields: name, email, password", ver, ka);
        }

        std::string name = body["name"];
        std::string email = body["email"];
        std::string password = body["password"];

        // Check duplicate email, then add
        auto [exists, added] = co_await net::co_spawn(
            pool,
            [&db, &email, &name, &password]() -> net::awaitable<std::pair<bool, bool>>
            {
                if (db.emailExists(email))
                    co_return std::pair{true, false};
                bool ok = db.addUser(name, email, password);
                co_return std::pair{false, ok};
            },
            net::use_awaitable);

        if (exists)
            co_return make_error(http::status::conflict,
                                 "Email already exists", ver, ka);
        if (!added)
            co_return make_error(http::status::internal_server_error,
                                 "Failed to create user", ver, ka);

        co_return make_json_response(http::status::created,
                                     json{{"message", "User created"}}, ver, ka);
    }

    // ---- routes with /users/{id} ----
    if (parts.size() == 2)
    {
        int id = parse_int(parts[1]);
        if (id < 0)
            co_return make_error(http::status::bad_request,
                                 "Invalid user ID", ver, ka);

        // GET /users/{id}
        if (method == http::verb::get)
        {
            json user = co_await net::co_spawn(
                pool,
                [&db, id]() -> net::awaitable<json>
                {
                    co_return db.getUserById(id);
                },
                net::use_awaitable);

            if (user.is_null())
                co_return make_error(http::status::not_found,
                                     "User not found", ver, ka);

            co_return make_json_response(http::status::ok, user, ver, ka);
        }

        // PUT /users/{id}
        if (method == http::verb::put)
        {
            json body;
            try
            {
                body = json::parse(req.body());
            }
            catch (...)
            {
                co_return make_error(http::status::bad_request,
                                     "Invalid JSON body", ver, ka);
            }

            if (!body.contains("name") || !body.contains("password"))
            {
                co_return make_error(http::status::bad_request,
                                     "Missing required fields: name, password", ver, ka);
            }

            std::string name = body["name"];
            std::string password = body["password"];

            bool ok = co_await net::co_spawn(
                pool,
                [&db, id, &name, &password]() -> net::awaitable<bool>
                {
                    co_return db.updateUser(id, name, password);
                },
                net::use_awaitable);

            if (!ok)
                co_return make_error(http::status::internal_server_error,
                                     "Failed to update user", ver, ka);

            co_return make_json_response(http::status::ok,
                                         json{{"message", "User updated"}}, ver, ka);
        }

        // DELETE /users/{id}
        if (method == http::verb::delete_)
        {
            bool ok = co_await net::co_spawn(
                pool,
                [&db, id]() -> net::awaitable<bool>
                {
                    co_return db.deleteUser(id);
                },
                net::use_awaitable);

            if (!ok)
                co_return make_error(http::status::internal_server_error,
                                     "Failed to delete user", ver, ka);

            co_return make_json_response(http::status::ok,
                                         json{{"message", "User deleted"}}, ver, ka);
        }
    }

    co_return make_error(http::status::method_not_allowed,
                         "Method not allowed", ver, ka);
}

// ---------------------------------------------------------------------------
// Route: /login
// ---------------------------------------------------------------------------

static net::awaitable<http::response<http::string_body>>
handle_login(const http::request<http::string_body> &req,
             DatabaseManager &db,
             net::thread_pool &pool)
{
    const unsigned ver = req.version();
    const bool ka = req.keep_alive();

    if (req.method() != http::verb::post)
        co_return make_error(http::status::method_not_allowed,
                             "Method not allowed", ver, ka);

    json body;
    try
    {
        body = json::parse(req.body());
    }
    catch (...)
    {
        co_return make_error(http::status::bad_request,
                             "Invalid JSON body", ver, ka);
    }

    if (!body.contains("email") || !body.contains("password"))
    {
        co_return make_error(http::status::bad_request,
                             "Missing required fields: email, password", ver, ka);
    }

    std::string email = body["email"];
    std::string password = body["password"];

    bool valid = co_await net::co_spawn(
        pool,
        [&db, &email, &password]() -> net::awaitable<bool>
        {
            co_return db.verifyLogin(email, password);
        },
        net::use_awaitable);

    if (!valid)
        co_return make_error(http::status::unauthorized,
                             "Invalid email or password", ver, ka);

    co_return make_json_response(http::status::ok,
                                 json{{"message", "Login successful"}}, ver, ka);
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
        co_return co_await handle_users(req, parts, db, pool);

    if (parts[0] == "login")
        co_return co_await handle_login(req, db, pool);

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

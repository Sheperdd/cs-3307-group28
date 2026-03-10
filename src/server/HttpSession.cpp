#include "HttpSession.h"
#include "http_utils.h"
#include "JwtManager.h"

// Endpoint handlers
#include "endpoints/Customers.h"
#include "endpoints/Appointments.h"
#include "endpoints/Jobs.h"
#include "endpoints/Mechanics.h"
#include "endpoints/Reviews.h"
#include "endpoints/Symptoms.h"
#include "endpoints/Vehicles.h"

#include <boost/asio/use_awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <iostream>

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

HttpSession::HttpSession(tcp::socket socket,
                         ServiceContext &ctx,
                         net::thread_pool &pool)
    : socket_(std::move(socket)), ctx_(ctx), pool_(pool)
{
    register_handlers();
}

// ---------------------------------------------------------------------------
// Handler registration
// ---------------------------------------------------------------------------

void HttpSession::register_handlers()
{
    handlers_["users"] = std::make_unique<CustomersHandler>();
    handlers_["auth"] = std::make_unique<CustomersHandler>();  // register + login
    handlers_["login"] = std::make_unique<CustomersHandler>(); // legacy alias
    handlers_["appointments"] = std::make_unique<AppointmentsHandler>();
    handlers_["jobs"] = std::make_unique<JobsHandler>();
    handlers_["mechanics"] = std::make_unique<MechanicsHandler>();
    handlers_["reviews"] = std::make_unique<ReviewsHandler>();
    handlers_["symptoms"] = std::make_unique<SymptomsHandler>();
    handlers_["vehicles"] = std::make_unique<VehiclesHandler>();
}

// ---------------------------------------------------------------------------
// Request router
// ---------------------------------------------------------------------------

net::awaitable<http::response<http::string_body>>
HttpSession::route_request(const http::request<http::string_body> &req)
{
    auto parts = http_utils::split_path(req.target());

    if (parts.empty())
        co_return http_utils::make_error(http::status::not_found,
                                         "Not found", req.version(), req.keep_alive());

    auto it = handlers_.find(parts[0]);
    if (it == handlers_.end())
        co_return http_utils::make_error(http::status::not_found,
                                         "Not found", req.version(), req.keep_alive());

    // ── Auth middleware ──────────────────────────────────────────────
    bool isPublic = false;

    // All /auth/* routes are public (register, login, logout)
    if (parts[0] == "auth")
        isPublic = true;

    // GET /mechanics (public search listing)
    if (parts[0] == "mechanics" && parts.size() == 1 && req.method() == http::verb::get)
        isPublic = true;

    std::optional<AuthInfo> auth;
    if (!isPublic)
    {
        auto token = http_utils::parse_cookie(req, "session_token");
        if (token.empty())
            co_return http_utils::make_error(http::status::unauthorized,
                                             "Authentication required",
                                             req.version(), req.keep_alive());

        auth = JwtManager::verifyToken(token);
        if (!auth.has_value())
            co_return http_utils::make_error(http::status::unauthorized,
                                             "Invalid or expired token",
                                             req.version(), req.keep_alive());
    }

    co_return co_await it->second->handle(req, parts, ctx_, pool_, auth);
}

// ---------------------------------------------------------------------------
// Main session loop  –  one per accepted connection
// ---------------------------------------------------------------------------

net::awaitable<void> HttpSession::run()
{
    auto self = shared_from_this(); // prevent destruction while coroutine is active
    try
    {
        for (;;)
        {
            // Read one full HTTP request
            http::request<http::string_body> req;
            co_await http::async_read(socket_, buffer_, req, net::use_awaitable);

            std::cout << "[HTTP] " << req.method_string() << " "
                      << req.target() << std::endl;

            // Route and build a response
            auto res = co_await route_request(req);

            // Send the response
            co_await http::async_write(socket_, res, net::use_awaitable);

            // If the client doesn't want keep-alive, close gracefully
            if (!res.keep_alive())
            {
                beast::error_code ec;
                socket_.shutdown(tcp::socket::shutdown_send, ec);
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

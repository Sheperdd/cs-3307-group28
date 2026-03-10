/**
 * @file Server.h
 * @brief Top-level HTTP server — owns the acceptor, DB, engine services,
 *        and thread pool; spawns HttpSession coroutines per connection.
 */
#pragma once

#include <boost/asio.hpp>
#include <boost/asio/thread_pool.hpp>

#include "../engine/DatabaseManager.h"
#include "../engine/RatingEngine.h"
#include "../engine/ProfitabilityEngine.h"
#include "../engine/DefaultCustomerValidator.h"
#include "../engine/CustomerService.h"
#include "../engine/MechanicService.h"
#include "../engine/AuthService.h"
#include "ServiceContext.h"

namespace net = boost::asio;
using tcp = net::ip::tcp;

/// @brief The main HTTP server class.  Listens for new connections on the
///        given port, spawns an HTTP-session coroutine for each one, and
///        owns the shared DatabaseManager, engine services, and DB thread pool.
class Server
{
public:
    /// @brief Constructs the server, opens the acceptor, and starts the
    ///        listener coroutine.
    /// @param io_context The io_context that drives asynchronous I/O.
    /// @param port       The TCP port to listen on.
    Server(net::io_context &io_context, short port);

private:
    /// @brief Coroutine that loops forever accepting new TCP connections.
    ///        For each accepted socket it co_spawns an http_session coroutine.
    net::awaitable<void> do_listen(tcp::acceptor acceptor);

    // --- Infrastructure ---
    DatabaseManager db_;                // The ONE database instance for the whole app
    net::thread_pool pool_{8};          // Thread pool for blocking DB work

    // --- Engine services (owned by Server, shared across sessions) ---
    RatingEngine           ratingEngine_;
    ProfitabilityEngine    profitabilityEngine_;
    DefaultCustomerValidator validator_;
    CustomerService        customerService_;
    MechanicService        mechanicService_;
    AuthService            authService_;

    // --- Service context passed to every session ---
    ServiceContext ctx_;
};

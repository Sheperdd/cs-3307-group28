#pragma once

#include <boost/asio.hpp>
#include <boost/asio/thread_pool.hpp>

#include "../engine/DatabaseManager.h"

namespace net = boost::asio;
using tcp = net::ip::tcp;

/// @brief The main HTTP server class.  Listens for new connections on the
///        given port, spawns an HTTP-session coroutine for each one, and
///        owns the shared DatabaseManager and DB thread pool.
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

    DatabaseManager db_;       // The ONE database instance for the whole app
    net::thread_pool pool_{8}; // Thread pool for blocking DB work
};

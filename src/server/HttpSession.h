#pragma once

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/beast.hpp>

#include "../engine/DatabaseManager.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

/// @brief Handles a single HTTP client connection as a coroutine.
///        Reads HTTP requests, routes them to the appropriate handler,
///        offloads blocking DB calls to a thread pool, and writes responses.
/// @param socket   The accepted TCP socket for this client.
/// @param db       Reference to the shared DatabaseManager instance.
/// @param pool     Reference to the thread pool used for blocking DB operations.
/// @return An awaitable<void> that completes when the connection is closed.
net::awaitable<void> http_session(tcp::socket socket,
                                  DatabaseManager &db,
                                  net::thread_pool &pool);

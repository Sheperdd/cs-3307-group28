#pragma once

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <string>
#include <unordered_map>

#include "ServiceContext.h"
#include "endpoints/EndpointHandler.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

/// @brief Manages a single HTTP client connection.
///        Owns the socket and buffer, reads requests in a loop,
///        routes them to registered EndpointHandler subclasses,
///        and writes responses back to the client.
class HttpSession : public std::enable_shared_from_this<HttpSession>
{
public:
  /// @brief Constructs the session, taking ownership of the socket.
  /// @param socket   The accepted TCP socket for this client.
  /// @param ctx      Reference to the shared ServiceContext (engine services + db).
  /// @param pool     Reference to the thread pool used for blocking DB operations.
  HttpSession(tcp::socket socket,
              ServiceContext &ctx,
              net::thread_pool &pool);

  /// @brief Starts the HTTP request/response loop as a coroutine.
  /// @return An awaitable<void> that completes when the connection is closed.
  net::awaitable<void> run();

private:
  /// @brief Routes an incoming request to the appropriate EndpointHandler.
  /// @param req The parsed HTTP request.
  /// @return An awaitable HTTP response.
  net::awaitable<http::response<http::string_body>>
  route_request(const http::request<http::string_body> &req);

  /// @brief Registers all endpoint handlers into the route map.
  void register_handlers();

  tcp::socket socket_;        ///< The client socket
  beast::flat_buffer buffer_; ///< Read buffer for this connection
  ServiceContext &ctx_;       ///< Shared service context (engine services + db)
  net::thread_pool &pool_;    ///< Thread pool for blocking DB work

  /// Maps URL prefixes (e.g. "users") to their endpoint handler
  std::unordered_map<std::string, std::unique_ptr<EndpointHandler>> handlers_;
};

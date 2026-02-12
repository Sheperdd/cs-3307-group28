#pragma once

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/beast.hpp>
#include <string>
#include <vector>

#include "../http_utils.h"
#include "../../engine/DatabaseManager.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

/// @brief Abstract base class for all REST endpoint handlers.
///        Each endpoint (e.g. Users, Vehicles) inherits from this
///        and implements the handle() coroutine to process requests
///        matching its URL prefix.
class EndpointHandler
{
public:
  virtual ~EndpointHandler() = default;

  /// @brief Processes an HTTP request routed to this endpoint.
  /// @param req         The incoming HTTP request.
  /// @param path_parts  The URL path split into segments (e.g. ["users", "42"]).
  /// @param db          Reference to the shared DatabaseManager.
  /// @param pool        Reference to the thread pool for blocking DB calls.
  /// @return An awaitable response to send back to the client.
  virtual net::awaitable<http::response<http::string_body>>
  handle(const http::request<http::string_body> &req,
         const std::vector<std::string> &path_parts,
         DatabaseManager &db,
         net::thread_pool &pool) = 0;
};

#pragma once

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/beast.hpp>
#include <string>
#include <vector>
#include <optional>

#include "../http_utils.h"
#include "../ServiceContext.h"
#include "../AuthInfo.h"
#include "DTOSerialization.h"
#include "RecordsSerialization.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class EndpointHandler
{
public:
  virtual ~EndpointHandler() = default;

  virtual net::awaitable<http::response<http::string_body>>
  handle(const http::request<http::string_body> &req,
         const std::vector<std::string> &path_parts,
         ServiceContext &ctx,
         net::thread_pool &pool,
         const std::optional<AuthInfo> &auth) = 0;
};

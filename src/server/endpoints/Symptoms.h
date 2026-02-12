#pragma once

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/beast.hpp>
#include <string>
#include <vector>

#include "../../engine/DatabaseManager.h"

namespace beast = boost::beast;
namespace http  = beast::http;
namespace net   = boost::asio;
using tcp = net::ip::tcp;

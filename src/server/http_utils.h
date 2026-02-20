#pragma once

#include <boost/beast.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>
#include <charconv>

namespace beast = boost::beast;
namespace http = beast::http;

using json = nlohmann::json;

namespace http_utils
{

  /// Splits a URI path like "/users/42" into segments: ["users", "42"]
  inline std::vector<std::string> split_path(std::string_view target)
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

  /// Try to parse a string as an int64_t (matching id types in DB). Returns -1 on failure.
  inline int64_t parse_int(const std::string &s)
  {
    int64_t val = -1;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), val);
    if (ec != std::errc{} || ptr != s.data() + s.size())
      return -1;
    return val;
  }

  /// Build a JSON HTTP response with the given status code and body.
  inline http::response<http::string_body>
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
  inline http::response<http::string_body>
  make_error(http::status status,
             const std::string &message,
             unsigned version,
             bool keep_alive)
  {
    return make_json_response(status, json{{"error", message}}, version, keep_alive);
  }

} // namespace http_utils

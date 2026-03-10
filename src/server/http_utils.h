/**
 * @file http_utils.h
 * @brief Inline HTTP helpers — path splitting, query-string parsing,
 *        JSON response builders, cookie extraction.
 */
#pragma once

#include <boost/beast.hpp>
#include <nlohmann/json.hpp>
#include <string>
#include <string_view>
#include <vector>
#include <charconv>
#include <unordered_map>

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

  /// Parse a query string (e.g. "specialty=brakes&maxDistanceKm=10") into a
  /// key→value map. Handles both the raw query portion and a full target URL.
  inline std::unordered_map<std::string, std::string>
  parse_query_params(std::string_view target)
  {
    std::unordered_map<std::string, std::string> params;
    auto qpos = target.find('?');
    if (qpos == std::string_view::npos)
      return params;
    std::string_view qs = target.substr(qpos + 1);
    while (!qs.empty())
    {
      auto amp = qs.find('&');
      std::string_view pair = (amp == std::string_view::npos) ? qs : qs.substr(0, amp);
      auto eq = pair.find('=');
      if (eq != std::string_view::npos)
        params.emplace(std::string(pair.substr(0, eq)),
                       std::string(pair.substr(eq + 1)));
      qs = (amp == std::string_view::npos) ? std::string_view{} : qs.substr(amp + 1);
    }
    return params;
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

  /// Extract a named cookie value from the request's Cookie header.
  /// Returns empty string if not found.
  inline std::string parse_cookie(
      const http::request<http::string_body> &req,
      const std::string &name)
  {
    auto it = req.find(http::field::cookie);
    if (it == req.end())
      return {};

    std::string_view cookies = it->value();
    size_t pos = 0;
    while (pos < cookies.size())
    {
      while (pos < cookies.size() && cookies[pos] == ' ')
        ++pos;

      auto semi = cookies.find(';', pos);
      std::string_view pair = cookies.substr(
          pos, semi == std::string_view::npos ? std::string_view::npos : semi - pos);

      auto eq = pair.find('=');
      if (eq != std::string_view::npos)
      {
        std::string_view cookieName = pair.substr(0, eq);
        if (cookieName == name)
          return std::string(pair.substr(eq + 1));
      }

      if (semi == std::string_view::npos)
        break;
      pos = semi + 1;
    }
    return {};
  }

} // namespace http_utils

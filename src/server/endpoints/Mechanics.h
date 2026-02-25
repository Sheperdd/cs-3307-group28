#pragma once

#include "EndpointHandler.h"

/// @brief Handles requests to /mechanics and /mechanics/{id}.
class MechanicsHandler : public EndpointHandler
{
public:
  net::awaitable<http::response<http::string_body>>
  handle(const http::request<http::string_body> &req,
         const std::vector<std::string> &path_parts,
         ServiceContext &ctx,
         net::thread_pool &pool) override;
};

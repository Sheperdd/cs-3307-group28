#pragma once

#include "EndpointHandler.h"

/// @brief Handles requests to /reviews and /reviews/{id}.
class ReviewsHandler : public EndpointHandler
{
public:
  net::awaitable<http::response<http::string_body>>
  handle(const http::request<http::string_body> &req,
         const std::vector<std::string> &path_parts,
         ServiceContext &ctx,
         net::thread_pool &pool) override;

private:
  // POST /reviews
  net::awaitable<http::response<http::string_body>>
  createReview(const http::request<http::string_body> &req, unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);
  // DELETE /reviews/{id}
  net::awaitable<http::response<http::string_body>>
  deleteReview(ReviewId reviewId, unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);
};

#include "Reviews.h"

net::awaitable<http::response<http::string_body>>
ReviewsHandler::handle(const http::request<http::string_body> &req,
                       const std::vector<std::string> &path_parts,
                       ServiceContext &ctx,
                       net::thread_pool &pool,
                       const std::optional<AuthInfo> &auth)
{
  const unsigned ver = req.version();
  const bool ka = req.keep_alive();

  // POST /reviews
  if (path_parts.size() == 1 && path_parts[0] == "reviews")
  {
    if (req.method() != http::verb::post)
      co_return http_utils::make_error(http::status::method_not_allowed,
                                       "Method not allowed", ver, ka);
    co_return co_await createReview(req, ver, ka, ctx, pool);
  }

  // DELETE /reviews/{id}
  if (path_parts.size() == 2 && path_parts[0] == "reviews")
  {
    if (req.method() != http::verb::delete_)
      co_return http_utils::make_error(http::status::method_not_allowed,
                                       "Method not allowed", ver, ka);
    ReviewId reviewId = http_utils::parse_int(path_parts[1]);
    if (reviewId < 0)
      co_return http_utils::make_error(http::status::bad_request,
                                       "Invalid review ID", ver, ka);
    co_return co_await deleteReview(reviewId, ver, ka, ctx, pool);
  }

  co_return http_utils::make_error(http::status::not_found,
                                   "Not found", ver, ka);
}

// POST /reviews
net::awaitable<http::response<http::string_body>>
ReviewsHandler::createReview(const http::request<http::string_body> &req,
                             unsigned ver, bool ka,
                             ServiceContext &ctx, net::thread_pool &pool)
{
  json body;
  bool parseOk = true;
  try
  {
    body = json::parse(req.body());
  }
  catch (...)
  {
    parseOk = false;
  }
  if (!parseOk)
    co_return http_utils::make_error(http::status::bad_request,
                                     "Invalid JSON body", ver, ka);

  ReviewCreate review;
  try
  {
    review = body.get<ReviewCreate>();
  }
  catch (const std::exception &e)
  {
    co_return http_utils::make_error(http::status::bad_request,
                                     std::string("Invalid review data: ") + e.what(), ver, ka);
  }

  struct Result
  {
    ReviewId reviewId{-1};
    std::string error;
  };

  auto res = co_await net::co_spawn(
      pool,
      [&ctx, review]() -> net::awaitable<Result>
      {
        Result r;
        try
        {
          r.reviewId = ctx.customerService.submitReview(review);
        }
        catch (const std::exception &e)
        {
          r.error = e.what();
        }
        co_return r;
      },
      net::use_awaitable);

  if (!res.error.empty())
    co_return http_utils::make_error(http::status::internal_server_error,
                                     res.error, ver, ka);

  co_return http_utils::make_json_response(http::status::created,
                                           json{{"reviewId", res.reviewId}}, ver, ka);
}

// DELETE /reviews/{id}
net::awaitable<http::response<http::string_body>>
ReviewsHandler::deleteReview(ReviewId reviewId, unsigned ver, bool ka,
                             ServiceContext &ctx, net::thread_pool &pool)
{
  struct Result
  {
    bool success{false};
    std::string error;
  };

  auto res = co_await net::co_spawn(
      pool,
      [&ctx, reviewId]() -> net::awaitable<Result>
      {
        Result r;
        try
        {
          r.success = ctx.customerService.deleteMyReview(reviewId);
        }
        catch (const std::exception &e)
        {
          r.error = e.what();
        }
        co_return r;
      },
      net::use_awaitable);

  if (!res.error.empty())
    co_return http_utils::make_error(http::status::internal_server_error,
                                     res.error, ver, ka);
  if (!res.success)
    co_return http_utils::make_error(http::status::not_found,
                                     "Review not found", ver, ka);

  co_return http_utils::make_json_response(http::status::ok,
                                           json{{"message", "Review deleted"}}, ver, ka);
}

#include "Jobs.h"

net::awaitable<http::response<http::string_body>>
JobsHandler::handle(const http::request<http::string_body> &req,
                    const std::vector<std::string> &path_parts,
                    ServiceContext &ctx,
                    net::thread_pool &pool)
{
  const unsigned ver = req.version();
  const bool ka = req.keep_alive();
  // TODO: Refactor to use ctx.mechanicService for job operations
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", req.version(), req.keep_alive());
}

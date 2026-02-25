#include "Mechanics.h"

net::awaitable<http::response<http::string_body>>
MechanicsHandler::handle(const http::request<http::string_body> &req,
                         const std::vector<std::string> &path_parts,
                         ServiceContext &ctx,
                         net::thread_pool &pool)
{
  // TODO: Refactor to use ctx.mechanicService for mechanic operations
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", req.version(), req.keep_alive());
}

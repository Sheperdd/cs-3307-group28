#include "Mechanics.h"

net::awaitable<http::response<http::string_body>>
MechanicsHandler::handle(const http::request<http::string_body> &req,
                         const std::vector<std::string> &path_parts,
                         DatabaseManager &db,
                         net::thread_pool &pool)
{
  // TODO: Implement mechanic CRUD logic
  co_return http_utils::make_error(http::status::not_implemented,
                                   "Not implemented", req.version(), req.keep_alive());
}

/**
 * @file Symptoms.cpp
 * @brief Symptom-form endpoint implementation — get, update, delete.
 */
#include "Symptoms.h"

net::awaitable<http::response<http::string_body>>
SymptomsHandler::handle(const http::request<http::string_body> &req,
                        const std::vector<std::string> &path_parts,
                        ServiceContext &ctx,
                        net::thread_pool &pool,
                        const std::optional<AuthInfo> &auth)
{
  const unsigned ver = req.version();
  const bool ka = req.keep_alive();

  SymptomFormId formId = http_utils::parse_int(path_parts[1]);
  if (formId < 0)
    co_return http_utils::make_error(http::status::bad_request,
                                     "Invalid symptom form ID", ver, ka);
  // Use a switch to determine which specific symptom-related operation to perform based on the URL path and HTTP method
  // Operations on specific symptom form ID
  switch (req.method())
  {
  case http::verb::get:
    co_return co_await getForm(formId, ver, ka, ctx, pool);
  case http::verb::patch:
    co_return co_await updateForm(formId, req, ver, ka, ctx, pool);
  case http::verb::delete_:
    co_return co_await deleteForm(formId, ver, ka, ctx, pool);
  default:
    co_return http_utils::make_error(http::status::method_not_allowed,
                                     "Method not allowed", ver, ka);
  }
}

// GET /symptoms/{id}
net::awaitable<http::response<http::string_body>>
SymptomsHandler::getForm(SymptomFormId formId, unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool)
{
  struct Result
  {
    std::optional<SymptomFormDTO> form;
    std::string error;
  };

  auto res = co_await net::co_spawn(
      pool,
      [&ctx, formId]() -> net::awaitable<Result>
      {
        Result r;
        try
        {
          r.form = ctx.customerService.getSymptomForm(formId);
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
  if (!res.form.has_value())
    co_return http_utils::make_error(http::status::not_found,
                                     "Symptom form not found", ver, ka);

  co_return http_utils::make_json_response(http::status::ok,
                                           json(*res.form), ver, ka);
}

// PATCH /symptoms/{id}
net::awaitable<http::response<http::string_body>>
SymptomsHandler::updateForm(SymptomFormId formId, const http::request<http::string_body> &req, unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool)
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

  SymptomFormUpdate formUpdate = body.get<SymptomFormUpdate>();
  // TODO: switch from SymptomFormUpdate to SymptomFormUpdateDTO once the former is implemented in CustomerService
  struct Result
  {
    bool success{false};
    std::string error;
  };

  auto res = co_await net::co_spawn(
      pool,
      [&ctx, formId, formUpdate]() -> net::awaitable<Result>
      {
        Result r;
        try
        {
          r.success = ctx.customerService.updateSymptomForm(formId, formUpdate);
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
                                     "Symptom form not found", ver, ka);

  co_return http_utils::make_json_response(http::status::ok,
                                           json{{"Symtom form updated", true}}, ver, ka);
}

// DELETE /symptoms/{id}
net::awaitable<http::response<http::string_body>>
SymptomsHandler::deleteForm(SymptomFormId formId, unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool)
{
  struct Result
  {
    bool success{false};
    std::string error;
  };

  auto res = co_await net::co_spawn(
      pool,
      [&ctx, formId]() -> net::awaitable<Result>
      {
        Result r;
        try
        {
          r.success = ctx.customerService.deleteSymptomForm(formId);
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
                                     "Symptom form not found", ver, ka);

  co_return http_utils::make_json_response(http::status::ok,
                                           json{{"Symtom form deleted", true}}, ver, ka);
}

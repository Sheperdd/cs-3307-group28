/**
 * @file Symptoms.h
 * @brief Handler for /symptoms/{id} — get, update, delete.
 */
#pragma once

#include "EndpointHandler.h"

/// @brief Handles requests to /symptoms and /symptoms/{id}.
class SymptomsHandler : public EndpointHandler
{
public:
    net::awaitable<http::response<http::string_body>>
    handle(const http::request<http::string_body> &req,
           const std::vector<std::string> &path_parts,
           ServiceContext &ctx,
           net::thread_pool &pool,
         const std::optional<AuthInfo> &auth) override;

private:
    // GET /symptoms/{id}
    net::awaitable<http::response<http::string_body>>
    getForm(SymptomFormId formId, unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);
    // PATCH /symptoms/{id}
    net::awaitable<http::response<http::string_body>>
    updateForm(SymptomFormId formId, const http::request<http::string_body> &req, unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);
    // DELETE /symptoms/{id}
    net::awaitable<http::response<http::string_body>>
    deleteForm(SymptomFormId formId, unsigned ver, bool ka, ServiceContext &ctx, net::thread_pool &pool);
};

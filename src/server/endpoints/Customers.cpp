#include "Customers.h"

// =====================================================================
//  Main router — dispatches to private sub-handler coroutines
// =====================================================================

net::awaitable<http::response<http::string_body>>
CustomersHandler::handle(const http::request<http::string_body> &req,
                         const std::vector<std::string> &path_parts,
                         ServiceContext &ctx,
                         net::thread_pool &pool)
{
    const unsigned ver = req.version();
    const bool ka = req.keep_alive();

    // ── POST /auth/register  or  /auth/login ────────────────────────
    if (path_parts.size() == 2 && path_parts[0] == "auth")
    {
        if (req.method() != http::verb::post)
            co_return http_utils::make_error(http::status::method_not_allowed,
                                             "Method not allowed", ver, ka);

        if (path_parts[1] == "register")
            co_return co_await registerUser(req, ver, ka, ctx, pool);
        else if (path_parts[1] == "login")
            co_return co_await loginUser(req, ver, ka, ctx, pool);
        else
            co_return http_utils::make_error(http::status::not_found,
                                             "Not found", ver, ka);
    }

    // All remaining routes require a numeric userId in path_parts[1]
    UserId userId = http_utils::parse_int(path_parts[1]);
    if (userId < 0)
        co_return http_utils::make_error(http::status::bad_request,
                                         "Invalid user ID", ver, ka);

    // ── GET | PATCH | DELETE  /users/{id} ────────────────────────────
    if (path_parts.size() == 2 && path_parts[0] == "users")
    {
        switch (req.method())
        {
        case http::verb::get:
            co_return co_await getUser(userId, ver, ka, ctx, pool);
        case http::verb::patch:
            co_return co_await updateUser(userId, req, ver, ka, ctx, pool);
        case http::verb::delete_:
            co_return co_await deleteUser(userId, ver, ka, ctx, pool);
        default:
            co_return http_utils::make_error(http::status::method_not_allowed,
                                             "Method not allowed", ver, ka);
        }
    }

    // ── PATCH /users/{id}/password ───────────────────────────────────
    if (path_parts.size() == 3 && path_parts[2] == "password")
    {
        if (req.method() != http::verb::patch)
            co_return http_utils::make_error(http::status::method_not_allowed,
                                             "Method not allowed", ver, ka);
        co_return co_await updatePassword(userId, req, ver, ka, ctx, pool);
    }

    // ── GET | POST  /users/{id}/vehicles ─────────────────────────────
    if (path_parts.size() == 3 && path_parts[2] == "vehicles")
    {
        switch (req.method())
        {
        case http::verb::get:
            co_return co_await getVehiclesForUser(userId, ver, ka, ctx, pool);
        case http::verb::post:
            co_return co_await addVehicleForUser(userId, req, ver, ka, ctx, pool);
        default:
            co_return http_utils::make_error(http::status::method_not_allowed,
                                             "Method not allowed", ver, ka);
        }
    }

    // ── GET /users/{id}/symptoms ─────────────────────────────────────
    if (path_parts.size() == 3 && path_parts[2] == "symptoms")
    {
        if (req.method() != http::verb::get)
            co_return http_utils::make_error(http::status::method_not_allowed,
                                             "Method not allowed", ver, ka);
        co_return co_await getSymptomFormsForUser(userId, ver, ka, ctx, pool);
    }

    // ── GET /users/{id}/appointments ─────────────────────────────────
    if (path_parts.size() == 3 && path_parts[2] == "appointments")
    {
        if (req.method() != http::verb::get)
            co_return http_utils::make_error(http::status::method_not_allowed,
                                             "Method not allowed", ver, ka);
        co_return co_await getAppointmentsForUser(userId, ver, ka, ctx, pool);
    }

    // ── GET /users/{id}/reviews ──────────────────────────────────────
    if (path_parts.size() == 3 && path_parts[2] == "reviews")
    {
        if (req.method() != http::verb::get)
            co_return http_utils::make_error(http::status::method_not_allowed,
                                             "Method not allowed", ver, ka);
        co_return co_await getReviewsByUser(userId, ver, ka, ctx, pool);
    }

    // ── Fallback — no route matched ──────────────────────────────────
    co_return http_utils::make_error(http::status::not_found,
                                     "Not found", ver, ka);
}

// =====================================================================
//  AUTH  (stubs — session / cookie design pending)
// =====================================================================

// POST /auth/register
// TODO: Implement when session/cookie design is finalized.
//       Will call ctx.db.createUser(), set session cookie, return AuthResult.
net::awaitable<http::response<http::string_body>>
CustomersHandler::registerUser(const http::request<http::string_body> &req,
                               unsigned ver, bool ka,
                               ServiceContext &ctx, net::thread_pool &pool)
{
    co_return http_utils::make_error(http::status::not_implemented,
                                     "Registration not implemented yet", ver, ka);
}

// POST /auth/login
// TODO: Implement when session/cookie design is finalized.
//       Will call ctx.db.verifyLogin() + ctx.db.getUserRecordByEmail(),
//       create a session, set cookie, return AuthResult.
net::awaitable<http::response<http::string_body>>
CustomersHandler::loginUser(const http::request<http::string_body> &req,
                            unsigned ver, bool ka,
                            ServiceContext &ctx, net::thread_pool &pool)
{
    co_return http_utils::make_error(http::status::not_implemented,
                                     "Login not implemented yet", ver, ka);
}

// =====================================================================
//  USER PROFILE  CRUD
// =====================================================================

// GET /users/{id}
net::awaitable<http::response<http::string_body>>
CustomersHandler::getUser(UserId userId, unsigned ver, bool ka,
                          ServiceContext &ctx, net::thread_pool &pool)
{
    struct Result
    {
        std::optional<CustomerDTO> user;
        std::string error;
    };

    auto res = co_await net::co_spawn(
        pool,
        [&ctx, userId]() -> net::awaitable<Result>
        {
            Result r;
            try
            {
                r.user = ctx.customerService.getCustomerProfile(userId);
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
    if (!res.user.has_value())
        co_return http_utils::make_error(http::status::not_found,
                                         "User not found", ver, ka);

    co_return http_utils::make_json_response(http::status::ok,
                                             json(*res.user), ver, ka);
}

// PATCH /users/{id}
net::awaitable<http::response<http::string_body>>
CustomersHandler::updateUser(UserId userId,
                             const http::request<http::string_body> &req,
                             unsigned ver, bool ka,
                             ServiceContext &ctx, net::thread_pool &pool)
{
    // TODO: Switch to ctx.customerService.updateCustomerProfile() when available
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

    UserUpdate updates = body.get<UserUpdate>();

    struct Result
    {
        bool ok{false};
        std::string error;
    };

    auto res = co_await net::co_spawn(
        pool,
        [&ctx, userId, updates]() -> net::awaitable<Result>
        {
            Result r;
            try
            {
                r.ok = ctx.db.updateUserRecord(userId, updates);
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
    if (!res.ok)
        co_return http_utils::make_error(http::status::not_found,
                                         "User not found or no changes applied", ver, ka);

    co_return http_utils::make_json_response(http::status::ok,
                                             json{{"message", "User updated"}}, ver, ka);
}

// DELETE /users/{id}
net::awaitable<http::response<http::string_body>>
CustomersHandler::deleteUser(UserId userId, unsigned ver, bool ka,
                             ServiceContext &ctx, net::thread_pool &pool)
{
    struct Result
    {
        bool ok{false};
        std::string error;
    };

    auto res = co_await net::co_spawn(
        pool,
        [&ctx, userId]() -> net::awaitable<Result>
        {
            Result r;
            try
            {
                r.ok = ctx.customerService.deleteCustomerProfile(userId);
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
    if (!res.ok)
        co_return http_utils::make_error(http::status::not_found,
                                         "User not found", ver, ka);

    co_return http_utils::make_json_response(http::status::ok,
                                             json{{"message", "User deleted"}}, ver, ka);
}

// PATCH /users/{id}/password
net::awaitable<http::response<http::string_body>>
CustomersHandler::updatePassword(UserId userId,
                                 const http::request<http::string_body> &req,
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

    if (!body.contains("password") || !body["password"].is_string())
        co_return http_utils::make_error(http::status::bad_request,
                                         "Missing 'password' field", ver, ka);

    std::string newPassword = body["password"].get<std::string>();

    struct Result
    {
        bool ok{false};
        std::string error;
    };

    auto res = co_await net::co_spawn(
        pool,
        [&ctx, userId, newPassword]() -> net::awaitable<Result>
        {
            Result r;
            try
            {
                r.ok = ctx.customerService.updateCustomerPasswordHash(userId, newPassword);
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
    if (!res.ok)
        co_return http_utils::make_error(http::status::not_found,
                                         "User not found", ver, ka);

    co_return http_utils::make_json_response(http::status::ok,
                                             json{{"message", "Password updated"}}, ver, ka);
}

// =====================================================================
//  VEHICLES  (per-user)
// =====================================================================

// GET /users/{id}/vehicles
net::awaitable<http::response<http::string_body>>
CustomersHandler::getVehiclesForUser(UserId userId, unsigned ver, bool ka,
                                     ServiceContext &ctx, net::thread_pool &pool)
{
    struct Result
    {
        std::vector<VehicleDTO> vehicles;
        std::string error;
    };

    auto res = co_await net::co_spawn(
        pool,
        [&ctx, userId]() -> net::awaitable<Result>
        {
            Result r;
            try
            {
                r.vehicles = ctx.customerService.listVehicles(userId);
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

    co_return http_utils::make_json_response(http::status::ok,
                                             json(res.vehicles), ver, ka);
}

// POST /users/{id}/vehicles
net::awaitable<http::response<http::string_body>>
CustomersHandler::addVehicleForUser(UserId userId,
                                    const http::request<http::string_body> &req,
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

    VehicleCreate vehicle;
    try
    {
        vehicle = body.get<VehicleCreate>();
    }
    catch (const std::exception &e)
    {
        co_return http_utils::make_error(http::status::bad_request,
                                         std::string("Bad vehicle payload: ") + e.what(),
                                         ver, ka);
    }

    struct Result
    {
        VehicleId id{-1};
        std::string error;
        bool badRequest{false};
    };

    auto res = co_await net::co_spawn(
        pool,
        [&ctx, userId, vehicle]() -> net::awaitable<Result>
        {
            Result r;
            try
            {
                r.id = ctx.customerService.addVehicle(userId, vehicle);
            }
            catch (const std::invalid_argument &e)
            {
                r.error = e.what();
                r.badRequest = true;
            }
            catch (const std::exception &e)
            {
                r.error = e.what();
            }
            co_return r;
        },
        net::use_awaitable);

    if (res.badRequest)
        co_return http_utils::make_error(http::status::bad_request,
                                         res.error, ver, ka);
    if (!res.error.empty())
        co_return http_utils::make_error(http::status::internal_server_error,
                                         res.error, ver, ka);

    co_return http_utils::make_json_response(http::status::created,
                                             json{{"vehicleId", res.id}}, ver, ka);
}

// =====================================================================
//  SYMPTOM FORMS  (per-user)
// =====================================================================

// GET /users/{id}/symptoms
net::awaitable<http::response<http::string_body>>
CustomersHandler::getSymptomFormsForUser(UserId userId, unsigned ver, bool ka,
                                         ServiceContext &ctx, net::thread_pool &pool)
{
    // TODO: Switch to ctx.customerService.listSymptomForms() when available
    struct Result
    {
        std::vector<SymptomFormDTO> forms;
        std::string error;
    };

    auto res = co_await net::co_spawn(
        pool,
        [&ctx, userId]() -> net::awaitable<Result>
        {
            Result r;
            try
            {
                r.forms = ctx.customerService.listSymptomForms(userId);
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

    co_return http_utils::make_json_response(http::status::ok,
                                             json(res.forms), ver, ka);
}

// =====================================================================
//  APPOINTMENTS  (per-user)
// =====================================================================

// GET /users/{id}/appointments
net::awaitable<http::response<http::string_body>>
CustomersHandler::getAppointmentsForUser(UserId userId, unsigned ver, bool ka,
                                         ServiceContext &ctx, net::thread_pool &pool)
{
    // TODO: Switch to ctx.customerService.listAppointments() when available
    struct Result
    {
        std::vector<AppointmentDTO> appointments;
        std::string error;
    };

    auto res = co_await net::co_spawn(
        pool,
        [&ctx, userId]() -> net::awaitable<Result>
        {
            Result r;
            try
            {
                r.appointments = ctx.customerService.listAppointments(userId);
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

    co_return http_utils::make_json_response(http::status::ok,
                                             json(res.appointments), ver, ka);
}

// =====================================================================
//  REVIEWS  (per-user)
// =====================================================================

// GET /users/{id}/reviews
net::awaitable<http::response<http::string_body>>
CustomersHandler::getReviewsByUser(UserId userId, unsigned ver, bool ka,
                                   ServiceContext &ctx, net::thread_pool &pool)
{
    // TODO: Switch to ctx.customerService.listMyReviews() when available
    struct Result
    {
        std::vector<ReviewDTO> reviews;
        std::string error;
    };

    auto res = co_await net::co_spawn(
        pool,
        [&ctx, userId]() -> net::awaitable<Result>
        {
            Result r;
            try
            {
                r.reviews = ctx.customerService.listMyReviews(userId);
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

    co_return http_utils::make_json_response(http::status::ok,
                                             json(res.reviews), ver, ka);
}

/*
// ---------------------------------------------------------------------------
// Route: /users  and  /users/{id}
// ---------------------------------------------------------------------------

/// Dispatches a request that targets /users or /users/{id}.
static net::awaitable<http::response<http::string_body>>
handle_users(const http::request<http::string_body> &req,
             const std::vector<std::string> &parts,
             DatabaseManager &db,
             net::thread_pool &pool)
{
    const auto method = req.method();
    const unsigned ver = req.version();
    const bool ka = req.keep_alive();

    // ---- GET /users ----
    if (parts.size() == 1 && method == http::verb::get)
    {
        json users = co_await net::co_spawn(
            pool,
            [&db]() -> net::awaitable<json>
            {
                co_return db.getAllUsers();
            },
            net::use_awaitable);

        co_return make_json_response(http::status::ok, users, ver, ka);
    }

    // ---- POST /users ----
    if (parts.size() == 1 && method == http::verb::post)
    {
        json body;
        try
        {
            body = json::parse(req.body());
        }
        catch (...)
        {
            co_return make_error(http::status::bad_request,
                                 "Invalid JSON body", ver, ka);
        }

        if (!body.contains("name") || !body.contains("email") || !body.contains("password"))
        {
            co_return make_error(http::status::bad_request,
                                 "Missing required fields: name, email, password", ver, ka);
        }

        std::string name = body["name"];
        std::string email = body["email"];
        std::string password = body["password"];

        // Check duplicate email, then add
        auto [exists, added] = co_await net::co_spawn(
            pool,
            [&db, &email, &name, &password]() -> net::awaitable<std::pair<bool, bool>>
            {
                if (db.emailExists(email))
                    co_return std::pair{true, false};
                bool ok = db.addUser(name, email, password);
                co_return std::pair{false, ok};
            },
            net::use_awaitable);

        if (exists)
            co_return make_error(http::status::conflict,
                                 "Email already exists", ver, ka);
        if (!added)
            co_return make_error(http::status::internal_server_error,
                                 "Failed to create user", ver, ka);

        co_return make_json_response(http::status::created,
                                     json{{"message", "User created"}}, ver, ka);
    }

    // ---- routes with /users/{id} ----
    if (parts.size() == 2)
    {
        int id = parse_int(parts[1]);
        if (id < 0)
            co_return make_error(http::status::bad_request,
                                 "Invalid user ID", ver, ka);

        // GET /users/{id}
        if (method == http::verb::get)
        {
            json user = co_await net::co_spawn(
                pool,
                [&db, id]() -> net::awaitable<json>
                {
                    co_return db.getUserById(id);
                },
                net::use_awaitable);

            if (user.is_null())
                co_return make_error(http::status::not_found,
                                     "User not found", ver, ka);

            co_return make_json_response(http::status::ok, user, ver, ka);
        }

        // PUT /users/{id}
        if (method == http::verb::put)
        {
            json body;
            try
            {
                body = json::parse(req.body());
            }
            catch (...)
            {
                co_return make_error(http::status::bad_request,
                                     "Invalid JSON body", ver, ka);
            }

            if (!body.contains("name") || !body.contains("password"))
            {
                co_return make_error(http::status::bad_request,
                                     "Missing required fields: name, password", ver, ka);
            }

            std::string name = body["name"];
            std::string password = body["password"];

            bool ok = co_await net::co_spawn(
                pool,
                [&db, id, &name, &password]() -> net::awaitable<bool>
                {
                    co_return db.updateUser(id, name, password);
                },
                net::use_awaitable);

            if (!ok)
                co_return make_error(http::status::internal_server_error,
                                     "Failed to update user", ver, ka);

            co_return make_json_response(http::status::ok,
                                         json{{"message", "User updated"}}, ver, ka);
        }

        // DELETE /users/{id}
        if (method == http::verb::delete_)
        {
            bool ok = co_await net::co_spawn(
                pool,
                [&db, id]() -> net::awaitable<bool>
                {
                    co_return db.deleteUser(id);
                },
                net::use_awaitable);

            if (!ok)
                co_return make_error(http::status::internal_server_error,
                                     "Failed to delete user", ver, ka);

            co_return make_json_response(http::status::ok,
                                         json{{"message", "User deleted"}}, ver, ka);
        }
    }

    co_return make_error(http::status::method_not_allowed,
                         "Method not allowed", ver, ka);
}

// ---------------------------------------------------------------------------
// Route: /login
// ---------------------------------------------------------------------------

static net::awaitable<http::response<http::string_body>>
handle_login(const http::request<http::string_body> &req,
             DatabaseManager &db,
             net::thread_pool &pool)
{
    const unsigned ver = req.version();
    const bool ka = req.keep_alive();

    if (req.method() != http::verb::post)
        co_return make_error(http::status::method_not_allowed,
                             "Method not allowed", ver, ka);

    json body;
    try
    {
        body = json::parse(req.body());
    }
    catch (...)
    {
        co_return make_error(http::status::bad_request,
                             "Invalid JSON body", ver, ka);
    }

    if (!body.contains("email") || !body.contains("password"))
    {
        co_return make_error(http::status::bad_request,
                             "Missing required fields: email, password", ver, ka);
    }

    std::string email = body["email"];
    std::string password = body["password"];

    bool valid = co_await net::co_spawn(
        pool,
        [&db, &email, &password]() -> net::awaitable<bool>
        {
            co_return db.verifyLogin(email, password);
        },
        net::use_awaitable);

    if (!valid)
        co_return make_error(http::status::unauthorized,
                             "Invalid email or password", ver, ka);

    co_return make_json_response(http::status::ok,
                                 json{{"message", "Login successful"}}, ver, ka);
}*/

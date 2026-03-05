#include "Jobs.h"

net::awaitable<http::response<http::string_body>>
JobsHandler::handle(const http::request<http::string_body> &req,
                    const std::vector<std::string> &path_parts,
                    ServiceContext &ctx,
                    net::thread_pool &pool)
{
  const unsigned ver = req.version();
  const bool ka = req.keep_alive();
  // Use a switch to determine which specific job-related operation to perform based on the URL path and HTTP method
  switch (path_parts.size())
  {
  case 2:
    if (req.method() == http::verb::get)
    {
      // GET /jobs/{id}
      JobId id = http_utils::parse_int(path_parts[1]);
      if (id < 0)
        co_return http_utils::make_error(http::status::bad_request,
                                         "Invalid Job ID", ver, ka);
      co_return co_await getJob(id, ver, ka, ctx, pool);
    }
    co_return http_utils::make_error(http::status::method_not_allowed,
                                     "Method not allowed", ver, ka);
  case 3:
  {
    JobId id = http_utils::parse_int(path_parts[1]);
    if (id < 0)
      co_return http_utils::make_error(http::status::bad_request,
                                       "Invalid Job ID", ver, ka);
    if (path_parts[2] == "stage" && req.method() == http::verb::put)
    {
      // PUT /jobs/{id}/stage
      co_return co_await updateStage(id, req, ver, ka, ctx, pool);
    }
    if (path_parts[2] == "complete" && req.method() == http::verb::post)
    {
      // POST /jobs/{id}/complete
      co_return co_await completeJob(id, req, ver, ka, ctx, pool);
    }
    if (path_parts[2] == "notes")
    {
      if (req.method() == http::verb::get)
      {
        // GET /jobs/{id}/notes
        co_return co_await getJobNotes(id, ver, ka, ctx, pool);
      }
      if (req.method() == http::verb::post)
      {
        // POST /jobs/{id}/notes
        co_return co_await addJobNote(id, req, ver, ka, ctx, pool);
      }
    }
    co_return http_utils::make_error(http::status::not_found,
                                     "Not found", ver, ka);
  }
  default:
    co_return http_utils::make_error(http::status::not_found,
                                     "Not found", ver, ka);
  }
}

// GET /jobs/{id}
net::awaitable<http::response<http::string_body>>
JobsHandler::getJob(JobId id, unsigned ver, bool ka,
                    ServiceContext &ctx, net::thread_pool &pool)
{
  struct Result
  {
    std::optional<JobDTO> job;
    std::string error;
  };

  auto res = co_await net::co_spawn(
      pool,
      [&ctx, id]() -> net::awaitable<Result>
      {
        Result r;
        try
        {
          r.job = ctx.mechanicService.getJob(id);
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
  if (!res.job.has_value())
    co_return http_utils::make_error(http::status::not_found,
                                     "Job not found", ver, ka);
  co_return http_utils::make_json_response(http::status::ok,
                                           json(*res.job), ver, ka);
}

// PUT /jobs/{id}/stage
net::awaitable<http::response<http::string_body>>
JobsHandler::updateStage(JobId id, const http::request<http::string_body> &req,
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
                                     "Invalid JSON", ver, ka);

  MechanicId mechanicId = body.value("mechanicId", MechanicId{0});
  int stageInt = body.value("stage", -1);
  int percentComplete = body.value("percentComplete", -1);
  std::string note = body.value("note", std::string{});

  if (mechanicId <= 0)
    co_return http_utils::make_error(http::status::bad_request,
                                     "Missing 'mechanicId'", ver, ka);
  if (stageInt < 0 || stageInt > 5)
    co_return http_utils::make_error(http::status::bad_request,
                                     "Invalid 'stage' (0-5)", ver, ka);
  if (percentComplete < -1 || percentComplete > 100)
    co_return http_utils::make_error(http::status::bad_request,
                                     "Invalid 'percentComplete' (-1 to 100)", ver, ka);

  auto stage = static_cast<JobStage>(stageInt);

  struct Result
  {
    bool ok{false};
    std::string error;
  };

  auto res = co_await net::co_spawn(
      pool,
      [&ctx, id, mechanicId, stage, percentComplete, &note]() -> net::awaitable<Result>
      {
        Result r;
        try
        {
          r.ok = ctx.mechanicService.updateJobStage(mechanicId, id, stage, percentComplete, note);
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
                                     "Job not found or not updated", ver, ka);
  json resp = {{"ok", true}};
  co_return http_utils::make_json_response(http::status::ok, resp, ver, ka);
}

// POST /jobs/{id}/complete
net::awaitable<http::response<http::string_body>>
JobsHandler::completeJob(JobId id, const http::request<http::string_body> &req,
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
                                     "Invalid JSON", ver, ka);

  MechanicId mechanicId = body.value("mechanicId", MechanicId{0});
  std::string note = body.value("note", std::string{});

  if (mechanicId <= 0)
    co_return http_utils::make_error(http::status::bad_request,
                                     "Missing 'mechanicId'", ver, ka);
  if (note.empty())
    co_return http_utils::make_error(http::status::bad_request,
                                     "A completion note is required", ver, ka);

  struct Result
  {
    bool ok{false};
    std::string error;
  };

  auto res = co_await net::co_spawn(
      pool,
      [&ctx, id, mechanicId, &note]() -> net::awaitable<Result>
      {
        Result r;
        try
        {
          r.ok = ctx.mechanicService.markJobComplete(mechanicId, id, note);
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
                                     "Job not found or not updated", ver, ka);
  json resp = {{"ok", true}};
  co_return http_utils::make_json_response(http::status::ok, resp, ver, ka);
}

// GET /jobs/{id}/notes
net::awaitable<http::response<http::string_body>>
JobsHandler::getJobNotes(JobId id, unsigned ver, bool ka,
                         ServiceContext &ctx, net::thread_pool &pool)
{
  struct Result
  {
    json notes;
    std::string error;
  };

  auto res = co_await net::co_spawn(
      pool,
      [&ctx, id]() -> net::awaitable<Result>
      {
        Result r;
        try
        {
          auto records = ctx.mechanicService.listJobNotes(id);
          r.notes = json(records);
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
                                           res.notes, ver, ka);
}

// POST /jobs/{id}/notes
net::awaitable<http::response<http::string_body>>
JobsHandler::addJobNote(JobId id, const http::request<http::string_body> &req,
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
                                     "Invalid JSON", ver, ka);

  std::string text = body.value("text", std::string{});
  MechanicId mechanicId = body.value("mechanicId", MechanicId{0});

  if (text.empty())
    co_return http_utils::make_error(http::status::bad_request,
                                     "Missing 'text' field", ver, ka);
  if (mechanicId <= 0)
    co_return http_utils::make_error(http::status::bad_request,
                                     "Missing 'mechanicId' field", ver, ka);

  struct Result
  {
    JobNoteId noteId{-1};
    std::string error;
  };

  auto res = co_await net::co_spawn(
      pool,
      [&ctx, id, mechanicId, &text]() -> net::awaitable<Result>
      {
        Result r;
        try
        {
          r.noteId = ctx.mechanicService.addJobNote(mechanicId, id, text);
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
  json resp = {{"noteId", res.noteId}};
  co_return http_utils::make_json_response(http::status::created,
                                           resp, ver, ka);
}

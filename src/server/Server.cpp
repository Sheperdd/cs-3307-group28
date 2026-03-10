/**
 * @file Server.cpp
 * @brief Server construction (binds acceptor, wires services) and the
 *        accept-loop coroutine.
 */
#include "Server.h"
#include "HttpSession.h"

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <iostream>
#include <memory>

Server::Server(net::io_context &io_context, short port)
    : ratingEngine_(db_, 30)                                              // 30-day half-life
    , profitabilityEngine_(db_, 80, 0.13)                                 // $80/hr default, 13% tax
    , customerService_(&db_, ratingEngine_, profitabilityEngine_, validator_)
    , mechanicService_(db_)
    , authService_(db_)
    , ctx_{ db_, customerService_, mechanicService_, authService_ }
{
    // Build the acceptor, bind, and listen – all before spawning the coroutine
    tcp::acceptor acceptor(io_context, {tcp::v4(), static_cast<net::ip::port_type>(port)});
    std::cout << "TorqueDesk REST API running on http://localhost:" << port << " ..." << std::endl;

    // Launch the listener coroutine on the io_context
    net::co_spawn(io_context,
                  do_listen(std::move(acceptor)),
                  net::detached);
}

net::awaitable<void> Server::do_listen(tcp::acceptor acceptor)
{
    for (;;)
    {
        // co_await a new connection
        tcp::socket socket = co_await acceptor.async_accept(net::use_awaitable);

        // Spawn an HTTP-session coroutine for this client
        auto session = std::make_shared<HttpSession>(
            std::move(socket), ctx_, pool_);

        net::co_spawn(
            acceptor.get_executor(),
            session->run(),
            net::detached);
    }
}

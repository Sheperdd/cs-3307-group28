#include "Server.h"
#include "Session.h"
#include <iostream>

// Main server stuff
Server::Server(boost::asio::io_context &io_context, short port) : acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
{
    std::cout << "TorqueDesk Server running on port " << port << "..." << std::endl;
    do_accept();
}

void Server::do_accept()
{
    // create a 'slot' type thing for a new socket, then wait for a connection
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket)
        {
            if (!ec)
            {
                // connection accepted so we can create a session for them
                Session::create(std::move(socket), db_)->start();
            }
            // Loop back and wait for the next person
            do_accept();
        });
}

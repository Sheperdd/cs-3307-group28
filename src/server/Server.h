#pragma once

#include <boost/asio.hpp>

#include "../engine/DatabaseManager.h"

using boost::asio::ip::tcp;

/// @brief The main server class. Listens for new connections and creates sessions for them. Also holds the database instance.
class Server
{
public:
    /// @brief Constructor for the server. Sets up the acceptor and starts listening for connections.
    /// @param io_context The io_context to use for asynchronous operations
    /// @param port The port to listen on
    Server(boost::asio::io_context &io_context, short port);

private:
    /// @brief The function that waits for new people (or connections). When a connection is accepted, it creates a new session for them and then loops back to wait for the next connection.
    void do_accept(); // The function that waits for new people (or connections)

    tcp::acceptor acceptor_; // The listening Socket for the server
    DatabaseManager db_;     // The ONE database instance for the whole app
};

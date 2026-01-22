#include "Server.h"
#include <iostream>

// Represents 1 connection with 1 client
// We use 'shared_from_this' so the session stays alive as long as the connection is open
class Session : public std::enable_shared_from_this<Session> {
public:
    Session(tcp::socket socket, DatabaseManager& db)
        : socket_(std::move(socket)), db_(db) {
    }

    void start() {
        do_read();
    }

private:
    void do_read() {
        auto self(shared_from_this());
        // Read data asynchronously (non-blocking)
        socket_.async_read_some(boost::asio::buffer(data_),
            [this, self](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    // Convert received data to string
                    std::string msg(data_, length);
                    std::cout << "[Client] " << msg << std::endl;

                    // TODO: Pass 'msg' to db_ logic here
                    // db_.processRequest(msg)...

                    // For now, just echo it back to prove it should work
                    do_write(length);
                }
            });
    }

    void do_write(std::size_t length) {
        auto self(shared_from_this());
        boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
            [this, self](boost::system::error_code ec, std::size_t /*length*/) {
                if (!ec) {
                    do_read(); // Wait for the next message from this client
                }
            });
    }

    tcp::socket socket_;
    DatabaseManager& db_; // Reference to the main DB
    char data_[1024];     // Buffer to hold incoming text
};

// Main server stuff
Server::Server(boost::asio::io_context& io_context, short port) : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    std::cout << "TorqueDesk Server running on port " << port << "..." << std::endl;
    do_accept();
}

void Server::do_accept() {
    // create a 'slot' type thing for a new socket, then wait for a connection
    acceptor_.async_accept(
        [this](boost::system::error_code ec, tcp::socket socket) {
            if (!ec) {
                // connection accepted so we can create a session for them
                std::make_shared<Session>(std::move(socket), db_)->start();
            }
            // Loop back and wait for the next person
            do_accept();
        });
}
#include <boost/asio.hpp>
#include <memory>
#include "../engine/DatabaseManager.h"

using boost::asio::ip::tcp;

class Server {
public:
    // Initialize server with an IO context and a port number
    Server(boost::asio::io_context& io_context, short port);

private:
	void do_accept(); // The function that waits for new people (or connections)

    tcp::acceptor acceptor_; // The listening Socket for the server
    DatabaseManager db_;     // The ONE database instance for the whole app
};
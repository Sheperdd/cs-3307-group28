#include <boost/asio.hpp>
#include "Server.h"
#include <iostream>

int main() {
    try {
        // The engine that drives the network events
        boost::asio::io_context io_context;

        // Create our Server on Port 6967
        Server s(io_context, 6967);

        // Run the loop forever
        io_context.run();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
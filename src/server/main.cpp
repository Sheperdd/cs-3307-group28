#include <boost/asio.hpp>
#include "Server.h"
#include <iostream>

/// @brief The main entry point for the TorqueDesk server application. Initializes the server and starts the I/O context to handle incoming client connections and requests.
/// @return an int value indicating the success or failure of the program execution.
int main()
{
    try
    {
        // The engine that drives the network events
        boost::asio::io_context io_context;

        // Create our Server on Port 6967
        Server s(io_context, 6967);

        // Run the loop forever
        io_context.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

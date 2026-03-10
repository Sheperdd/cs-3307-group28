/**
 * @file main.cpp
 * @brief Entry point for the TorqueDesk REST API server.
 */
#include <boost/asio.hpp>
#include "Server.h"
#include <iostream>

/// @brief Creates the io_context, instantiates Server, and runs the event loop.
///        Creates the io_context, instantiates the Server (which starts
///        the coroutine-based HTTP listener), and runs the event loop.
/// @return 0 on clean exit, non-zero on unhandled exception.
int main()
{
    try
    {
        boost::asio::io_context io_context;

        // Create the HTTP server on port 6967
        Server s(io_context, 6967);

        // Run the event loop (blocks until all work is done)
        io_context.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}

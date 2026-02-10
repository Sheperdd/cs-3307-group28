#pragma once

#include <boost/asio.hpp>
#include <iostream>

#include "../engine/DatabaseManager.h"

using boost::asio::ip::tcp;

// Represents 1 connection with 1 client
// We use 'shared_from_this' so the session stays alive as long as the connection is open

/// @brief Handles a single client connection, allowing for asynchronous reading and writing of data.
class Session : public std::enable_shared_from_this<Session>
{
public:
  // we use a shared_ptr to keep the Session object alive during asynchronous operations
  typedef std::shared_ptr<Session> pointer;

  /// @brief  Creates a new Session instance for a given socket and database reference.
  /// @param socket The TCP socket representing the client's connection.
  /// @param db A reference to the DatabaseManager instance for handling database operations.
  /// @return A shared pointer to the newly created Session instance.
  static pointer create(tcp::socket socket, DatabaseManager &db);

  /// @brief Retrieves the socket associated with this session.
  /// @return A reference to the tcp::socket used for communication with the client.
  tcp::socket &getSocket();

  /// @brief Starts the session by initiating the asynchronous read operation.
  void start();

private:
  /// @brief Constructs a Session object with the provided socket and database reference.
  /// @param socket The TCP socket representing the client's connection.
  /// @param db A reference to the DatabaseManager instance for handling database operations.
  Session(tcp::socket socket, DatabaseManager &db)
      : socket_(std::move(socket)), db_(db) {}

  /// @brief Initiates an asynchronous read operation to receive data from the client. Upon receiving data, it processes the message and prepares for the next read operation.
  void do_read();

  /// @brief Initiates an asynchronous write operation to send data back to the client. After writing, it prepares for the next read operation.
  /// @param length The length of the data to be written back to the client.
  void do_write(std::size_t length);

  tcp::socket socket_;  // The socket for this client connection
  DatabaseManager &db_; // Reference to the main DB
  char data_[1024];     // Buffer to hold incoming text
};

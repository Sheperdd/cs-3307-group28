#include "Session.h"

Session::pointer Session::create(tcp::socket socket, DatabaseManager &db)
{
  // We use 'new' here because we want to return a shared_ptr, and 'make_shared' doesn't work with private constructors
  return pointer(new Session(std::move(socket), db));
}

tcp::socket &Session::getSocket()
{
  // This function isn't actually used in our current code, but it could be useful if we want to do something with the socket outside of the Session class
  return socket_;
}

void Session::start()
{
  // Start the asynchronous read loop for this session
  do_read();
}

void Session::do_read()
{
  auto self(shared_from_this());
  // Read data asynchronously (non-blocking)
  socket_.async_read_some(boost::asio::buffer(data_),
                          [this, self](boost::system::error_code ec, std::size_t length)
                          {
                            if (!ec)
                            {
                              // Convert received data to string
                              std::string msg(data_, length);
                              std::cout << "[Client] " << msg << std::endl;

                              // TODO: Pass 'msg' to db_ logic here
                              // db_.processRequest(msg)...

                              // For now, just echo it back to prove it should work
                              do_write(length);
                            }
                            else
                            {
                              std::cerr << "Error: " << ec.message() << std::endl;
                            }
                          });
}

void Session::do_write(std::size_t length)
{
  auto self(shared_from_this());
  boost::asio::async_write(socket_, boost::asio::buffer(data_, length),
                           [this, self](boost::system::error_code ec, std::size_t /*length*/)
                           {
                             if (!ec)
                             {
                               do_read(); // Wait for the next message from this client
                             }
                             else
                             {
                               std::cerr << "Error: " << ec.message() << std::endl;
                             }
                           });
}

#include "connection.hpp"

namespace http {

connection::connection(boost::asio::io_service &io_service)
  : socket_(io_service)
{}

connection::~connection()
{}

connection::streambuf_type &
connection::io_buffer()
{ return iobuf_; }

connection::socket_type &
connection::socket()
{ return socket_; }

} // namespace http

#include "connection.hpp"
#include "connection_manager.hpp"

//#include <iostream>

namespace http {

connection::connection(
  boost::asio::io_service &io_service,
  connection::OWNER o
  )
  : socket_(io_service),
    owner_(o)
{}

connection::~connection()
{}

connection::OWNER 
connection::owner() const
{ return owner_; }

void
connection::owner(connection::OWNER o)
{ 
  owner_ = o; 
}

void connection::close()
{
  socket_.close();  
}

connection::streambuf_type &
connection::io_buffer()
{ return iobuf_; }

connection::socket_type &
connection::socket()
{ return socket_; }

bool connection::is_open() const
{ return socket_.is_open(); }

} // namespace http

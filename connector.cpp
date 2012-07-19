#include "connector.hpp"
#include <boost/bind.hpp>
#include <iostream>

namespace http { 

using asio::ip::tcp;

connector::connector(
  asio::io_service& io_service,
  tcp::resolver &resolver)
: resolver_(resolver)
{}

void connector::async_resolve_and_connect(
  tcp::socket &socket,
  std::string const& server,
  std::string const& service,
  connector::handler_t handler)
{
  handler_ = handler;
  tcp::resolver::query query(server, service);
  resolver_.async_resolve(
    query,
    boost::bind(
      &connector::handle_resolve,
      this,
      boost::ref(socket),
      asio::placeholders::error,
      asio::placeholders::iterator
      )
    );
}


void connector::handle_resolve(
  tcp::socket &socket,
  boost::system::error_code const &err,
  tcp::resolver::iterator endpoint_iterator)
{
  if(!err){
    asio::async_connect(
      socket,
      endpoint_iterator,
      boost::bind(
        handler_,
        asio::placeholders::error
        )
      );
  }else{
    std::cout << "Error: " << err.message() << "\n";
  }
}

} // namespace http


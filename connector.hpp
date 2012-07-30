#ifndef GAISWT_CONNECTOR_HPP_
#define GAISWT_CONNECTOR_HPP_

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <string>

namespace http {

namespace asio = boost::asio;

struct connector : boost::noncopyable
{
  typedef boost::asio::ip::tcp tcp;

  typedef boost::function<
      void(boost::system::error_code const&)
      > handler_t; 

  connector(asio::io_service &io_service,
            asio::ip::tcp::resolver &resolver);

  void async_resolve_and_connect(
    asio::ip::tcp::socket &socket,
    std::string const& server,
    std::string const& service,
    handler_t handler);

  void handle_resolve(
    tcp::socket &socket,
    boost::system::error_code const &err,
    tcp::resolver::iterator endpoint_iterator);
  
  tcp::resolver &resolver_;
  handler_t handler_;
};

} // namespace http
#endif

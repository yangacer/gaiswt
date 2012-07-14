#ifndef HTTP_CLIENT_HPP
#define HTTP_CLIENT_HPP

#include <string>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include "coroutine.hpp"

namespace http {

using boost::asio::ip::tcp;

class client : coroutine
{
public:
  typedef void result_type;

  client(
    boost::asio::io_service& io_service,
    const std::string& server, 
    const std::string& service, 
    const std::string& path);
  
  void handle_resolve(const boost::system::error_code& err,
                  tcp::resolver::iterator endpoint_iterator);
  
  void operator()(boost::system::error_code err = boost::system::error_code(),
                  std::size_t length = 0);

  boost::shared_ptr<tcp::resolver> resolver_;
  boost::shared_ptr<tcp::socket> socket_;
  boost::shared_ptr<boost::asio::streambuf> request_;
  boost::shared_ptr<boost::asio::streambuf> response_;
};

} // namespace http

#endif

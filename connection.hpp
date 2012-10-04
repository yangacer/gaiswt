#ifndef GAISWT_CONNECTION_HPP_
#define GAISWT_CONNECTION_HPP_

#include <boost/asio/io_service.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/shared_ptr.hpp>

namespace http {

class connection {
public:
  
  typedef boost::asio::streambuf streambuf_type;
  typedef boost::asio::ip::tcp::socket socket_type;

  connection(boost::asio::io_service &io_service);
  ~connection();
  
  streambuf_type &io_buffer();
  socket_type &socket();

private:
  streambuf_type iobuf_;
  socket_type socket_;
};

typedef boost::shared_ptr<connection> connection_ptr;

} // namespace http

#endif // header guard

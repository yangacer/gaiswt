#ifndef GAISWT_CONNECTION_HPP_
#define GAISWT_CONNECTION_HPP_

#include <boost/asio/io_service.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace http {

class connection_manager;

class connection 
: public boost::enable_shared_from_this<connection>,
  private boost::noncopyable
{
public:
  enum OWNER { SERVER = 0, AGENT };

  typedef boost::asio::streambuf streambuf_type;
  typedef boost::asio::ip::tcp::socket socket_type;

  connection(
    boost::asio::io_service &io_service, 
    connection_manager &cm,
    OWNER owner);

  ~connection();
  
  streambuf_type &io_buffer();
  socket_type &socket();
  OWNER owner() const;
  void owner(OWNER o);
  void close();

private:
  streambuf_type iobuf_;
  socket_type socket_;
  connection_manager &connection_manager_;
  OWNER owner_;
};

typedef boost::shared_ptr<connection> connection_ptr;

} // namespace http

#endif // header guard

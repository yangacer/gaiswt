#ifndef GAISWT_SERVER_HPP_
#define GAISWT_SERVER_HPP_

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <string>
#include <boost/noncopyable.hpp>
#include "connection.hpp"
#include "uri_dispatcher.hpp"
#include "server_handler.hpp"
#include "mmstore.hpp"

namespace http {

class connection_manager;

class server
: private boost::noncopyable
{
public:
  explicit server(
    boost::asio::io_service &io_service,
    connection_manager &cm,
    mmstore &mms,
    std::string const& document_root);

  ~server();

  uri_dispatcher &get_uri_dispatcher();
  
  void run(
    std::string const &address,
    std::string const &port);

private:
  void start_accept();
  void handle_accept(const boost::system::error_code& e);
  void handle_stop();

  boost::asio::io_service &io_service_;
  boost::asio::signal_set signals_;
  boost::asio::ip::tcp::acceptor acceptor_;
  connection_manager &connection_manager_;
  connection_ptr connection_ptr_;
  uri_dispatcher dispatcher_;
  server_handler default_handler_;

};

} // namespace http

#endif // header guard

#ifndef GAISWT_SERVER_HPP_
#define GAISWT_SERVER_HPP_

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <string>
#include <boost/noncopyable.hpp>
#include "connection.hpp"
#include "interface.hpp"
#include "entity.hpp"

namespace http {

class connection_manager;

namespace parser{
  template<typename T> struct request_first_line;
  template<typename T> struct header_list;
} // namespace parser

class server
: public interface::concrete_interface,
  private boost::noncopyable
{
public:
  explicit server(const std::string& address, const std::string& port,
      const std::string& doc_root);

  // void run();

private:
  void start_accept();
  void handle_accept(const boost::system::error_code& e);
  void handle_read_status_line();
  void handle_read_headers();
  void handle_stop();

  boost::asio::io_service &io_service_;
  boost::asio::signal_set signals_;
  boost::asio::ip::tcp::acceptor acceptor_;
  connection_manager &connection_manager_;
  connection_ptr connection_ptr_;
  entity::request request_;
  boost::asio::deadline_timer deadline_;
  bool stop_;
};

} // namespace http

#endif // header guard

#ifndef GAISWT_SERVER_HPP_
#define GAISWT_SERVER_HPP_

#include <boost/asio.hpp>
#include <string>
#include <boost/noncopyable.hpp>
#include "connection.hpp"
#include "connection_manager.hpp"
#include "interface.hpp"
#include "entity.hpp"

namespace http {

class server
: private boost::noncopyable
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
  //request_handler request_handler_;
};

} // namespace http

#endif // header guard

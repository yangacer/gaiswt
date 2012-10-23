#ifndef GAISWT_SESSION_HPP_
#define GAISWT_SESSION_HPP_
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include "connection.hpp"
#include "entity.hpp"

namespace http {
 
class uri_dispatcher;

class session
: public boost::enable_shared_from_this<session>
{
public:
  session(
    boost::asio::io_service &io_service, 
    connection_manager &cm,
    connection_ptr c,
    uri_dispatcher &dispatcher);

  ~session();

  void handle_read_status_line(boost::system::error_code const &err);
  void handle_read_headers(boost::system::error_code const &err);

private:
  void notify(boost::system::error_code const &err);
  void check_deadline();

  connection_manager &connection_manager_;
  connection_ptr connection_ptr_;
  entity::request request_;
  boost::asio::deadline_timer deadline_;
  bool stop_check_deadline_;
  uri_dispatcher &dispatcher_;
};

} // namespace http

#endif // header guard


#ifndef GAISWT_SESSION_HPP_
#define GAISWT_SESSION_HPP_
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include "connection.hpp"
#include "entity.hpp"

namespace http {
  
class session
: public boost::enable_shared_from_this<session>
{
public:
  session(boost::asio::io_service &io_service, connection_ptr c);
  ~session();
  void handle_read_status_line();
  void handle_read_headers();
private:
  connection_ptr connection_ptr_;
  entity::request request_;
  boost::asio::deadline_timer deadline_;
  bool stop_check_deadline_;
};

} // namespace http

#endif // header guard


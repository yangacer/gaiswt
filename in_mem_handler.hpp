#ifndef GAISWT_IN_MEM_HANDLER_HPP_
#define GAISWT_IN_MEM_HANDLER_HPP_

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/shared_ptr.hpp>
#include "entity.hpp"
#include "handler.hpp"
#include "connection.hpp"

namespace http {
 
struct in_memory_handler 
: handler
{
  OBSERVER_INSTALL_LOG_REQUIRED_INTERFACE_;

  in_memory_handler(mode_t mode, boost::uint32_t transfer_timeout_sec=10);
  virtual ~in_memory_handler();

  void on_response(
    boost::system::error_code const &err,
    http::entity::response const &response,
    http::connection_ptr conn);

  void on_request(
    boost::system::error_code const &err,
    http::entity::request const &request,
    http::connection_ptr conn);

protected:
  void start_transfer();
  void handle_transfer(boost::system::error_code const &err, boost::uint32_t length);
  void handle_timeout();

  boost::uint32_t transfer_timeout_;
  boost::shared_ptr<boost::asio::deadline_timer> deadline_ptr_;
};


} // namespace http

#endif

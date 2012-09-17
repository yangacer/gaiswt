#ifndef GAISWT_IN_MEM_HANDLER_HPP_
#define GAISWT_IN_MEM_HANDLER_HPP_

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/shared_ptr.hpp>
#include "entity.hpp"
#include "handler.hpp"
#include "connection.hpp"

namespace http {
 
struct save_in_memory 
: handler_interface::concrete_interface
{
  OBSERVER_INSTALL_LOG_REQUIRED_INTERFACE_;

  save_in_memory(boost::asio::streambuf &buffer);
  virtual ~save_in_memory();

  void on_response(
    http::entity::response const &response,
    http::connection const& connection_incoming);

  void preprocess_error(boost::system::error_code const &err );

private:

  void handle_read(boost::system::error_code err);
  
  boost::shared_ptr<connection> connection_ptr_;
  boost::asio::streambuf &buffer_;
};


} // namespace http

#endif

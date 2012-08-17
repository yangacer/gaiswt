#ifndef GAISWT_IN_MEM_HANDLER_HPP_
#define GAISWT_IN_MEM_HANDLER_HPP_

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include "entity.hpp"
#include "handler.hpp"

namespace http {
 
struct save_in_memory 
: handler_interface::concrete_interface
{
  OBSERVER_INSTALL_LOG_REQUIRED_INTERFACE_;

  save_in_memory(boost::asio::streambuf &buffer);

  void on_response(
    http::entity::response const &response, 
    boost::asio::ip::tcp::socket &socket, 
    boost::asio::streambuf &front_data);

  void preprocess_error(boost::system::error_code err );

private:

  void handle_read(boost::system::error_code err);

  boost::asio::ip::tcp::socket *socket_;
  boost::asio::streambuf &buffer_;
};


} // namespace http

#endif

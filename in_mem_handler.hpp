#ifndef GAISWT_IN_MEM_HANDLER_HPP_
#define GAISWT_IN_MEM_HANDLER_HPP_

#include <boost/asio/ip/tcp.hpp>
#include <boost/shared_ptr.hpp>
#include "entity.hpp"
#include "handler.hpp"
#include "connection.hpp"

namespace http {
 
struct in_memory_handler 
: handler_interface::concrete_interface
{
  OBSERVER_INSTALL_LOG_REQUIRED_INTERFACE_;

  enum mode_t { read =0, write};

  in_memory_handler(mode_t mode);
  virtual ~in_memory_handler();

  void on_response(
    boost::system::error_code const &err,
    http::entity::response const &response,
    http::connection_ptr conn);

  void on_request(
    boost::system::error_code const &err,
    http::entity::request const &request,
    http::connection_ptr conn);

  /*
  template<typename F, typename O, typename ...Args>
  void on_response(F f, O&& o, Args&& ...args)
  { 
    handler_interface::on_response::attach(
      f, o, std::forward<Arg>(args));
  }
  */


protected:
  void start_transfer();
  void handle_transfer(boost::system::error_code const &err, boost::uint32_t length);
  void notify(boost::system::error_code const &err);

private:
  mode_t mode_;
  http::connection_ptr connection_ptr_;
  entity::request const *request_;
  entity::response const *response_;
};


} // namespace http

#endif

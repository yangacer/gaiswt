#ifndef GAISWT_HANDLER_HPP_
#define GAISWT_HANDLER_HPP_

#include <boost/system/error_code.hpp>
#include "observer/observable.hpp"
#include "agent.hpp"
#include "entity.hpp"
#include "connection.hpp"

namespace http {

enum MORE_DATA{ NOMORE=0, MORE };

namespace handler_interface {

  typedef observer::observable<
    void(
      boost::system::error_code const &,
      entity::response const&, 
      http::connection_ptr,
      MORE_DATA)
    > on_response;

  typedef observer::observable<
    void(
      boost::system::error_code const &,
      entity::request const&, 
      http::connection_ptr,
      MORE_DATA)
    > on_request;

  typedef observer::make_observable<
    observer::vector<
      on_response,
      on_request
      >
    >::base concrete_interface;
} // namespace handler_interface

struct handler
: handler_interface::concrete_interface
{
  enum mode_t { read=0, write };

  OBSERVER_INSTALL_LOG_REQUIRED_INTERFACE_;

  handler(mode_t mode);
  virtual ~handler();

  void on_response(
    boost::system::error_code const &err,
    http::entity::response const &response,
    http::connection_ptr conn);

  void on_request(
    boost::system::error_code const &err,
    http::entity::request const &request,
    http::connection_ptr conn);
  
  mode_t mode() const;
  connection_ptr connection();
  entity::response const& response() const;
  entity::request const& request() const;

protected:
  void notify(boost::system::error_code const &err);

private:
  mode_t mode_;
  connection_ptr connection_ptr_;
  entity::request const* request_;
  entity::response const* response_;
};

} // namespace http
#endif //header guard

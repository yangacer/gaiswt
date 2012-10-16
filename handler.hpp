#ifndef GAISWT_HANDLER_HPP_
#define GAISWT_HANDLER_HPP_

#include "observer/observable.hpp"
#include "agent.hpp"
#include "entity.hpp"

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

} // namespace http
#endif //header guard

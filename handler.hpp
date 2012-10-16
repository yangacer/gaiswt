#ifndef GAISWT_HANDLER_HPP_
#define GAISWT_HANDLER_HPP_

#include "observer/observable.hpp"
#include "agent.hpp"
#include "entity.hpp"

namespace http {

namespace handler_interface {
 
  typedef observer::observable<
    void(boost::system::error_code const&,
         entity::request const &,
         connection_ptr)
    > 
    on_request;
 
  typedef observer::observable<
    void(boost::system::error_code const&,
         entity::response const &,
         connection_ptr)
    > 
    on_response;

  typedef observer::make_observable<
    observer::vector<
      on_response,
      on_request
      >
    >::base concrete_interface;
} // namespace handler_interface

} // namespace http
#endif //header guard

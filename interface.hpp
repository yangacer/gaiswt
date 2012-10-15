#ifndef GAISWT_INTERFACE_HPP_
#define GAISWT_INTERFACE_HPP

#include "observer/observable.hpp"
#include "connection.hpp"
#include "entity.hpp"

namespace http {

namespace interface {
  
  typedef observer::observable<
    void(
      boost::system::error_code const &,
      entity::response const&, 
      http::connection_ptr)
    > on_response;

  typedef observer::observable<
    void(
      boost::system::error_code const &,
      entity::request const&, 
      http::connection_ptr)
    > on_request;
    

  typedef observer::make_observable<
      observer::vector<
        on_response,
        on_request
      >
    >::base concrete_interface;

}} // namespace http::interface


#endif // header guard

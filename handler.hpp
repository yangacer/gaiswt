#ifndef GAISWT_HANDLER_HPP_
#define GAISWT_HANDLER_HPP_

#include "observer/observable.hpp"
#include "agent.hpp"

namespace http {

namespace handler_interface {
  typedef observer::observable<void()> complete;
  typedef observer::observable<void(boost::system::error_code const&)> error;

  typedef observer::make_observable<
      observer::vector<
        complete, error
      >
    >::base concrete_interface;
} // namespace handler_interface

} // namespace http
#endif //header guard

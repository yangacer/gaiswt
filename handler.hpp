#ifndef GAISWT_HANDLER_HPP_
#define GAISWT_HANDLER_HPP_

#include "observer/observable.hpp"
#include "agent2.hpp"

namespace http {


namespace handler_interface {
  typedef observer::observable<void()> complete;
  typedef observer::observable<void(boost::system::error_code)> error;

  typedef observer::make_observable<
      observer::vector<
        complete, error
      >
    >::base concrete_interface;
} // namespace handler_interface

namespace handler_helper {

template<typename Handler>
void connect_agent_handler(http::agent &agent, Handler *handler)
{
  namespace ph = std::placeholders;

  agent.http::agent_interface::ready_for_read::attach_mem_fn(
    &Handler::on_response, handler, ph::_1, ph::_2, ph::_3);

  agent.http::agent_interface::error::attach_mem_fn(
    &Handler::preprocess_error, handler, ph::_1);
}

} // namespace handler_helper

} // namespace http
#endif //header guard

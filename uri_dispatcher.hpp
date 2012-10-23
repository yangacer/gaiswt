#ifndef GAISWT_URI_DISPATCHER_HPP_
#define GAISWT_URI_DISPATCHER_HPP_

#include "interface.hpp"
#include <stdexcept>
#include <map>
#include <functional>
#include <string>
#include <ostream>

namespace http {

class uri_dispatcher 
{
  struct observer_t : interface::on_request
  {
    using interface::on_request::callback_class;
  };

  typedef observer_t::callback_class handler_t;

public:
  
  /** Get observer container for attach handler.
   * @param uri URI prefix to be observed.
   * @return Observer container corresponds to the uri parameter.
   */
  observer_t &operator[](std::string const& uri);

  template<typename ...Args>
  void attach(std::string const& uri, Args&&...args)
  {
    namespace ph = std::placeholders;
    handlers_[uri].attach(std::forward<Args>(args)..., ph::_1, ph::_2, ph::_3);
  }

  template<typename ...Args>
  void detach(std::string const& uri, Args&&...args)
  {
    auto target = handlers_.find(uri);
    if(target == handlers_.end()) return;
    target->second.detach(std::forward<Args>(args)...);
  }

  void detach_all();

  /** Get observer container for notify handlers.
   * @param uri Full uri.
   * @return Observer container that is matched with the uri parameter.
   * @throw Throw out_of_range when uri does not match any observer
   * container.
   */
  observer_t &operator()(std::string const& uri);

private:
  std::map<std::string, observer_t, std::greater<std::string> >
   handlers_;
};

} // namespace http

#endif

#ifndef GAISWT_URI_DISPATCHER_HPP_
#define GAISWT_URI_DISPATCHER_HPP_

#include "handler.hpp"
#include <stdexcept>
#include <map>
#include <functional>
#include <string>
#include <ostream>

namespace http {

class uri_dispatcher 
{
  struct observer_t : handler_interface::on_request
  {
    using handler_interface::on_request::callback_class;
  };

  typedef observer_t::callback_class handler_t;

public:

  observer_t &operator[](std::string const& uri);
  observer_t &operator()(std::string const& uri);
  
  //void dump(std::ostream &os, std::string const& uri);

private:
  std::map<std::string, observer_t, std::greater<std::string> >
   handlers_;
};

} // namespace http

#endif

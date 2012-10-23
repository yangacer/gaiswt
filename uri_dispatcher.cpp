#include "uri_dispatcher.hpp"

namespace http {

// -------- uri_dispatcher impl --------

uri_dispatcher::observer_t &
uri_dispatcher::operator[](std::string const& uri)
{
  return handlers_[uri];
}

uri_dispatcher::observer_t &
uri_dispatcher::operator()(std::string const& uri)
{
  auto target = handlers_.lower_bound( uri);
  if(target != handlers_.end() && 0 == uri.find(target->first))
      return target->second;
  throw std::out_of_range("Unregistered URI");
}

void uri_dispatcher::detach_all()
{
  for(auto i=handlers_.begin();i!=handlers_.end();i++)
    i->second.detach_all();
}

} // namespace http


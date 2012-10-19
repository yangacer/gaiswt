#include "dispatcher.hpp"

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

/*
void uri_dispatcher::dump(std::ostream &os, std::string const& uri)
{
  auto end = handlers_.upper_bound(uri);
  auto i = handlers_.lower_bound(uri);
  while(i != handlers_.end()){
    os << i->first << "\n";
    if(i==end)
      break;
      ++i;
  }
}
*/

} // namespace http


#include "header.hpp"

namespace http {

std::vector<entity::field>::iterator 
find_header(std::vector<entity::field>& headers, std::string const& name)
{
  for(auto i = headers.begin(); i != headers.end(); ++i)
    if(i->name == name)
      return i;
  return headers.end();
}

} // namespace http

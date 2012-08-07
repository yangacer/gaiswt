#include "utility.hpp"
#include "entity.hpp"
#include <stdexcept>
#include <boost/lexical_cast.hpp>

namespace http {

std::string 
determine_service(entity::url const & url)
{
  return (url.port) ?
    boost::lexical_cast<std::string>(url.port) :
    url.scheme
    ;
}

} // namespace http

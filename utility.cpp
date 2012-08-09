#include "utility.hpp"
#include "entity.hpp"
#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/asio/buffer.hpp>

namespace http {

std::string 
determine_service(entity::url const & url)
{
  return (url.port) ?
    boost::lexical_cast<std::string>(url.port) :
    url.scheme
    ;
}

boost::asio::mutable_buffer 
make_buffer(std::pair<void*, boost::uint32_t> region)
{
  return boost::asio::mutable_buffer(region.first, region.second);
}

} // namespace http

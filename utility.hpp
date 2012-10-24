#ifndef GAISWT_UTILITY_HPP_
#define GAISWT_UTILITY_HPP_

#include <string>
#include <utility>
#include <boost/cstdint.hpp>

namespace boost { namespace asio {
  class mutable_buffer;
}} // namespace boost::asio

namespace http{
namespace entity {
  struct url;
} // namespace entity

std::string determine_service(entity::url const & url);

boost::asio::mutable_buffer 
make_buffer(std::pair<void*, boost::uint32_t> region);

} // namespace http

#endif // header guard

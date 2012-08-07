#ifndef GAISWT_UTILITY_HPP_
#define GAISWT_UTILITY_HPP_

#include <string>

namespace http{
namespace entity {
  struct url;
} // namespace entity

std::string determine_service(entity::url const & url);

} // namespace http

#endif // header guard

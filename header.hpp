//
// header.hpp
// ~~~~~~~~~~
//
// Copyright (c) 2003-2012 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef GAISWT_HEADER_HPP_
#define GAISWT_HEADER_HPP_

#include <string>
#include "entity.hpp"

namespace http {
namespace server4 {

struct header
{
  std::string name;
  std::string value;
};

} // namespace server4



} // namespace http

#endif // HTTP_SERVER4_HEADER_HPP

#ifndef GAISWT_GENERATOR_DEF_HPP_
#define GAISWT_GENERATOR_DEF_HPP_

#include "generator.hpp"

namespace http{ namespace generator {
    
template<typename Iterator>
url_esc_string<Iterator>::url_esc_string()
: url_esc_string::base_type(start)
{
  start =
    *( karma::char_("a-zA-Z0-9_.~") |
       karma::char_('-') |
       ('%' << hex)
     )
    ;
}

} // namespace generator
} // namespace http

#endif // header guard

#ifndef GAISWT_GENERATOR_DEF_HPP_
#define GAISWT_GENERATOR_DEF_HPP_

#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/fusion/container/map.hpp>
#include <boost/fusion/include/adapt_adt.hpp>
#include "generator.hpp"
#include "fusion_adt.hpp"


BOOST_FUSION_ADAPT_ADT(
  http::entity::uri,
  (std::string, std::string, obj.path, /**/)
  (bool, bool, obj.query_map.empty(), /**/)
  (http::entity::query_map_t, http::entity::query_map_t, obj.query_map, /**/)
  )

namespace http{ namespace generator {

template<typename Iterator>
url_esc_string<Iterator>::url_esc_string()
: url_esc_string::base_type(start)
{
  using karma::char_;
  using karma::eps;
  using karma::_r1;

  start =
    *( char_("a-zA-Z0-9_.~") |
       char_('-') |
       ( eps(_r1) << char_('/')) |
       ('%' << hex)
     )
    ;
}

template<typename Iterator>
uri<Iterator>::uri()
: uri::base_type(start)
{
  query_value =
    karma::double_ | int64_ | esc_string(ESCAPE_PATH_DELIM)
    ;
  
  query_pair =
    esc_string(ESCAPE_PATH_DELIM) << '=' <<
    query_value
    ;

  query_map =
    query_pair % '&'
    ;

  start =
    esc_string(RESERVE_PATH_DELIM) <<
    // false if query_map is not empty
    (&karma::false_ <<  '?' << query_map) |  
    karma::omit[ query_map ]
    ;
}

} // namespace generator
} // namespace http

#endif // header guard

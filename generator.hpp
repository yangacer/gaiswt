#ifndef GAISWT_GENERATOR_HPP_
#define GAISWT_GENERATOR_HPP_

#include <boost/spirit/include/karma.hpp>
#include "entity.hpp"

namespace http {
namespace generator {

namespace karma = boost::spirit::karma;


enum path_delim_option{ 
  ESCAPE_PATH_DELIM = false, 
  RESERVE_PATH_DELIM = true
};

template<typename Iterator>
struct url_esc_string
: karma::grammar<Iterator, std::string(path_delim_option)>
{
  url_esc_string();
  karma::rule<Iterator, std::string(path_delim_option)> start;
  karma::uint_generator<unsigned char, 16> hex;
};

template<typename Iterator>
struct uri
: karma::grammar<Iterator, entity::uri()>
{
  uri();
  karma::rule<Iterator, entity::uri()> start;
  karma::rule<Iterator, entity::query_value_t()> query_value;
  karma::rule<Iterator, std::pair<std::string, entity::query_value_t>()> query_pair;
  karma::rule<Iterator, entity::query_map_t()> query_map;
  url_esc_string<Iterator> esc_string;
  karma::int_generator< boost::int64_t > int64_;
};



} // namespace generator
} // namespace http

#endif // header guard

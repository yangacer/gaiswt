#ifndef GAISWT_GENERATOR_HPP_
#define GAISWT_GENERATOR_HPP_

#include <boost/spirit/include/karma.hpp>

namespace http {
namespace generator {

namespace karma = boost::spirit::karma;

template<typename Iterator>
struct url_esc_string
  : karma::grammar<Iterator, std::string()>
{
  url_esc_string();
  karma::rule<Iterator, std::string()> start;
  karma::uint_generator<unsigned char, 16> hex;
};

} // namespace generator
} // namespace http

#endif // header guard

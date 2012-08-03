#ifndef GAISWT_PARSER_HPP_
#define GAISWT_PARSER_HPP_

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include "entity.hpp"

namespace http {

namespace qi = boost::spirit::qi;

template<typename Iterator>
struct field_parser
: qi::grammar<Iterator, entity::field()>
{
  field_parser();
  qi::rule<Iterator, entity::field()> start;
};

template<typename Iterator>
struct response_parser
: qi::grammar<Iterator, entity::response()>
{
  response_parser();
  qi::rule<Iterator, entity::response()> start;
  field_parser<Iterator> field_rule;
};


} // namespace http

#endif // header guard

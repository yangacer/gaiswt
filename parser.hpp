#ifndef GAISWT_PARSER_HPP_
#define GAISWT_PARSER_HPP_

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include "entity.hpp"

namespace http {

namespace parser {

namespace qi = boost::spirit::qi;
using qi::phrase_parse;

template<typename Iterator>
struct field
: qi::grammar<Iterator, entity::field()>
{
  field();
  qi::rule<Iterator, entity::field()> start;
};


template<typename Iterator>
struct header_list
: qi::grammar<Iterator, std::vector<entity::field>()>
{
  header_list();
  qi::rule<Iterator, std::vector<entity::field>()> start;
  field<Iterator> field_rule;
};

template<typename Iterator>
struct response_first_line
: qi::grammar<Iterator, entity::response()>
{
  response_first_line();
  qi::rule<Iterator, entity::response()> start;
};


}} // namespace http::parser

#endif // header guard

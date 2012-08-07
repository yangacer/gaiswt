#ifndef GAISWT_PARSER_HPP_
#define GAISWT_PARSER_HPP_

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/support_istream_iterator.hpp>
#include "entity.hpp"

namespace http {
namespace parser {

namespace qi = boost::spirit::qi;

//using qi::phrase_parse;
//using qi::space;
using boost::spirit::istream_iterator;

template<typename Iterator>
struct url_esc_string
: qi::grammar<Iterator, std::string(char const*)>
{
  url_esc_string();
  qi::rule<Iterator, char()> unesc_char;
  qi::rule<Iterator, std::string(char const*)> start;
  qi::uint_parser<unsigned char, 16, 2, 2> hex2;
};

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

template<typename Iterator>
struct uri
: qi::grammar<Iterator, entity::uri()>
{
  uri();
  qi::rule<Iterator, entity::uri()> start;
  qi::rule<Iterator, entity::query_value_t()> query_value;
  qi::rule<Iterator, std::pair<std::string, entity::query_value_t>()> query_pair;
  qi::rule<Iterator, entity::query_map_t()> query_map;
  url_esc_string<Iterator> esc_string;
};

template<typename Iterator>
struct url
: qi::grammar<Iterator, entity::url()>
{
  url();
  qi::rule<Iterator, entity::url()> start;
  uri<Iterator> query;
};

#define GEN_PARSE_FN(Parser) \
  template<typename Iter, typename Struct> \
  bool parse_##Parser(Iter beg, Iter end, Struct &obj) \
  { \
    static Parser<Iter> parser; \
    return qi::phrase_parse( \
      beg, end, parser, qi::space, obj); \
  } 

GEN_PARSE_FN(response_first_line)
GEN_PARSE_FN(header_list)
GEN_PARSE_FN(uri)
GEN_PARSE_FN(url)

}} // namespace http::parser

#endif // header guard

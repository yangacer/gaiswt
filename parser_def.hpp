#ifndef GAISWT_PARSER_DEF_HPP_
#define GAISWT_PARSER_DEF_HPP_

#include "parser.hpp"
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <iostream>

#ifdef GAISWT_DEBUG_PARSER  
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>

namespace phoenix = boost::phoenix;
#endif

BOOST_FUSION_ADAPT_STRUCT(
  http::entity::field,
  (std::string, name)
  (std::string, value)
  )

BOOST_FUSION_ADAPT_STRUCT(
  http::entity::response,
  (int, http_version_major)
  (int, http_version_minor)
  (unsigned int, status_code)
  (std::string, message)
  //(std::vector<http::entity::field>, headers)
  )

namespace http { namespace parser {

template<typename Iterator>
field<Iterator>::field()
: field::base_type(start)
{
  using qi::char_;
  using qi::lit;

  char const cr('\r');

  start %= 
    +(char_ - ':') >> lit(": ") >>  
    +(char_ - cr);

#ifdef GAISWT_DEBUG_PARSER  
  using phoenix::val;
  using phoenix::construct;
  using namespace qi::labels;

  start.name("field");

  qi::on_error<qi::fail>
    ( start ,
      std::cout<<
      val("Error! Expecting ")<<
      _4<<
      val(" here: ")<<
      construct<std::string>(_3,_2)<<
      std::endl
    );
  
  debug(start);
#endif
}

template<typename Iterator>
header_list<Iterator>::header_list()
: header_list::base_type(start)
{
  using qi::lit;
  char const *crlf("\r\n");

  start %=
    +(field_rule >> lit(crlf))
      ;

#ifdef GAISWT_DEBUG_PARSER  
  using phoenix::val;
  using phoenix::construct;
  using namespace qi::labels;

  start.name("response");
  field_rule.name("field");

  qi::on_error<qi::fail>
    ( start ,
      std::cout<<
      val("Error! Expecting ")<<
      _4<<
      val(" here: ")<<
      construct<std::string>(_3,_2)<<
      std::endl
    );

  qi::debug(start);
#endif
}

template<typename Iterator>
response_first_line<Iterator>::response_first_line()
: response_first_line::base_type(start)
{
  using qi::char_;
  using qi::lit;
  using qi::int_;
  using qi::uint_;

  char const cr('\r'), sp(' ');
  char const *crlf("\r\n");

  start %= 
    lit("HTTP/") >> int_ >> '.' >> int_ >> sp >>
    uint_ >> sp >>
    +(char_ - cr) >> lit(crlf)
    ;

#ifdef GAISWT_DEBUG_PARSER  
  using phoenix::val;
  using phoenix::construct;
  using namespace qi::labels;

  start.name("response");

  qi::on_error<qi::fail>
    ( start ,
      std::cout<<
      val("Error! Expecting ")<<
      _4<<
      val(" here: ")<<
      construct<std::string>(_3,_2)<<
      std::endl
    );

  qi::debug(start);
#endif
}

}} // namespace http::parser

#endif // header guard

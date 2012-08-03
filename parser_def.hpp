#ifndef GAISWT_PARSER_DEF_HPP_
#define GAISWT_PARSER_DEF_HPP_

#include "parser.hpp"
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>
#include <iostream>

#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>


namespace phoenix = boost::phoenix;

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
  (std::vector<http::entity::field>, headers)
  )

namespace http {

template<typename Iterator>
field_parser<Iterator>::field_parser()
: field_parser::base_type(start)
{
  using qi::char_;
  using qi::lit;
  using phoenix::val;
  using phoenix::construct;
  using namespace qi::labels;

  char const cr('$');

  start %= 
    +(char_ - ':') >> lit(": ") >>  
    +(char_ - cr);

#ifdef GAISWT_DEBUG_PARSER  
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
response_parser<Iterator>::response_parser()
: response_parser::base_type(start)
{
  using qi::char_;
  using qi::lit;
  using qi::int_;
  using qi::uint_;
  using phoenix::val;
  using phoenix::construct;
  using namespace qi::labels;

  char const cr('$'), sp(' ');
  char const *crlf("$*");

  start %= 
    lit("HTTP/") >> int_ >> '.' >> int_ >> sp >>
    uint_ >> sp >>
    +(char_ - cr) >> lit(crlf) >>
    +(field_rule >> lit(crlf)) >> 
    lit(crlf)
    ;

#ifdef GAISWT_DEBUG_PARSER  
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

} // namespace htp

#endif // header guard

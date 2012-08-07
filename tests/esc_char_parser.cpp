#define GAISWT_DEBUG_PARSER
#include "parser_def.hpp"

int main()
{
  namespace qi = http::parser::qi;
  namespace ph = boost::phoenix;

  http::parser::url_esc_string<std::string::iterator> esc_str;

  std::string input = "abc%5b%e8%a8%b1%e5%8a%9f%e8%93%8b%5def$", unesc_output;
  std::string::iterator beg(input.begin()), end(input.end());
  char const* delim = "$";

  if(!qi::parse(beg, end,  esc_str(delim), unesc_output))
    std::cout << "Parsing failed\n";
  
  std::cout << unesc_output << "\n";

  return 0;
}

#define GAISWT_DEBUG_PARSER
#include "parser_def.hpp"
#include <iostream>

/** test data
 * HTTP/1.1 200 OK\r\ntest: 1234\r\nacer: 5678\r\n\r\n
 *
 * */

int main(int argc, char **argv)
{
  using namespace http;

  typedef parser::istream_iterator iter_t;

  // read from stdin
  std::cin.unsetf(std::ios::skipws);
  iter_t beg(std::cin), end;

  http::entity::response rep;
  
  if(!parser::parse_response_first_line(beg, end, rep)){
    std::cerr << "Parsing of 'response first line' is failed\n";
    return 1;
  }
  
  // std::cout << "Consumed bytes: " << std::cin.tellg() << "\n"; 

  if(!parser::parse_header_list(beg, end, rep.headers))
    std::cerr << "Parsing of 'header list' is failed\n";
  else
    std::cout << rep << "\n";
 
  std::cout << "Not parsed\n";
  std::string tmp;
  while(getline(std::cin, tmp)){
    std::cout << tmp << "\n";
  }

  return 0;
}

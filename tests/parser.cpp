#include "parser_def.hpp"
#include <iostream>

/** test data
 * HTTP/1.1 200 OK\r\ntest: 1234\r\nacer: 5678\r\n\r\n
 *
 * */

int main(int argc, char **argv)
{
  using namespace http;

  typedef std::string::iterator iter_t;

  std::string input = argv[1];
  parser::response_first_line<iter_t> response_first_line;
  parser::header_list<iter_t> header_list;
  iter_t beg = input.begin(), end = input.end();
  http::entity::response rep;
  
  if(!parser::phrase_parse(
      beg, end,
      response_first_line, 
      parser::qi::space,
      rep)){
    std::cerr << "Parsing of 'response first line' is failed\n";
    return 1;
  }
  
  std::cout << "Consumed bytes: " << beg - input.begin() << "\n"; 

  if(!parser::phrase_parse(
      beg, end,
      header_list,
      parser::qi::space,
      rep.headers))
    std::cerr << "Parsing of 'header list' is failed\n";
  else
    std::cout << rep << "\n";
  
  return 0;
}

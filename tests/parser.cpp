#include "parser_def.hpp"
#include <iostream>

int main(int argc, char **argv)
{
  std::string input = argv[1];
  http::response_parser<std::string::iterator> parser;
  http::entity::response rep;
  
  if(!http::qi::phrase_parse(
      input.begin(), 
      input.end(),
      parser, 
      http::qi::space,
      rep))
    std::cerr << "Parse failed\n";
  else
    std::cout << rep << "\n";
}

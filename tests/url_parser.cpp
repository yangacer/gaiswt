#define GAISWT_DEBUG_PARSER
#include "parser_def.hpp"
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
  if(argc != 2){
    std::cerr << "Usage: url_parser <url>\n";
    std::cerr << "  e.g. url_parser 'http://yangacer.twbbs.org/~yangacer'\n";
    return 1;
  }

  std::string input = argv[1];  
  auto beg(input.begin()), end(input.end());
  http::entity::url url;

  if(!http::parser::parse_url(beg, end,  url)){
    std::cerr << "Parsing failed\n";  
  }
    

  return 0;
}

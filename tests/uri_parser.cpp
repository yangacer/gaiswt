#define GAISWT_DEBUG_PARSER
#include "parser_def.hpp"
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
  if(argc != 2){
    std::cerr << "Usage: uri_parser <uri>\n";
    std::cerr << "  e.g. uri_parser '/path?oid=123&val=acer'\n";
    return 1;
  }

  std::string input = argv[1];  
  auto beg(input.begin()), end(input.end());
  http::entity::uri uri;

  if(!http::parser::parse_uri(beg, end, uri)) {
    std::cerr << "Parsing failed\n";  
  }
    

  return 0;
}

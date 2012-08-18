#include "parser.hpp"

int main()
{
  std::string input = "GET /test/abc%5b%e8%a8%b1%e5%8a%9f%e8%93%8b%5def HTTP/1.1\r\n";
  http::entity::request req;
  auto beg(input.begin()), end(input.end());

  if(!http::parser::parse_request_first_line(beg, end, req)){
    std::cout << "Parsing failed\n";
  }

  std::cout << 
    "method: " << req.method << "\n" <<
    "uri: " << req.query.path << "\n"
    ;

  return 0;
}

#include "entity.hpp"
#include "generator.hpp"

namespace http {
namespace entity {

field::field(std::string name, std::string value)
  : name(name), value(value)
{}

request
request::stock_request(stock_request_t type)
{
  request rt;
  switch(type){
  case GET_PAGE:
    rt.method = "GET";
    rt.http_version_major =
      rt.http_version_minor = 1;
    rt.headers.emplace_back("Accept", "*/*");
    rt.headers.emplace_back("Connection", "close");
    break;
  default:
    break;
  }
  return rt;
}

} // namespace entity

std::vector<entity::field>::iterator 
find_header(std::vector<entity::field>& headers, std::string const& name)
{
  for(auto i = headers.begin(); i != headers.end(); ++i)
    if(i->name == name)
      return i;
  return headers.end();
}

} // namespace http

std::ostream & operator << (std::ostream &os, http::entity::field const &f)
{
  os << f.name << ": " << f.value << "\r\n";
  return os;
}

std::ostream & operator << (std::ostream &os, http::entity::request const &req)
{
  http::generator::ostream_iterator out(os);
  http::generator::generate_request(out, req);
  return os;
}

std::ostream & operator << (std::ostream &os, http::entity::response const &rep)
{
  http::generator::ostream_iterator out(os);
  http::generator::generate_response(out, rep);
  return os;
}


#include "entity.hpp"

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

}} // namespace entity

std::ostream & operator << (std::ostream &os, http::entity::field const &f)
{
  os << f.name << ": " << f.value << "\r\n";
  return os;
}

std::ostream & operator << (std::ostream &os, http::entity::request const &req)
{
  char const sp(' ');
  char const *crlf("\r\n");

  os << req.method << sp << req.uri << sp;
  os << "HTTP/" << 
    req.http_version_major << "." <<
    req.http_version_minor << crlf; 

  for(auto i = req.headers.begin(); i != req.headers.end(); ++i)
    os << *i;
  os << crlf;
  return os;
}

std::ostream & operator << (std::ostream &os, http::entity::response const &rep)
{
  char const sp(' ');
  char const *crlf("\r\n");

  os << "HTTP/" << 
    rep.http_version_major << "." << 
    rep.http_version_minor << sp;
    
  os << rep.status_code << sp;

  os << rep.message << crlf;

  for(auto i = rep.headers.begin(); i != rep.headers.end(); ++i)
    os << *i;
  os << crlf;

  return os;
}


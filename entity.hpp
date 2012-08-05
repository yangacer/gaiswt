#ifndef GAISWT_ENTITY_HPP_
#define GAISWT_ENTITY_HPP_

#include <string>
#include <ostream>
#include <vector>

namespace http {
namespace entity {


struct field
{
  field(){}
  field(std::string name, std::string value);
  std::string name, value;
};

struct request
{
  enum stock_request_t {
    GET_PAGE = 0
  };

  std::string method, uri;
  
  int http_version_major,
      http_version_minor;

  std::vector<field> headers;

  static request stock_request(stock_request_t type);
};

struct response
{
  int http_version_major,
      http_version_minor;

  unsigned int status_code;
  std::string message;
  std::vector<field> headers;

};

}} // namespace http::entity

std::ostream & operator << (std::ostream &os, http::entity::field const &f);
std::ostream & operator << (std::ostream &os, http::entity::request const &req);
std::ostream & operator << (std::ostream &os, http::entity::response const &rep);

#endif // header guard

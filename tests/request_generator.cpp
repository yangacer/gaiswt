#include "generator_def.hpp"
#include "entity.hpp"

int main()
{
  using namespace http;

  entity::request req;

  req = entity::request::stock_request(entity::request::GET_PAGE);

  req.query.path = "/";

  std::cout << req ;

}

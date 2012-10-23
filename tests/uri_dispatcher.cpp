#include "uri_dispatcher.hpp"
#include <cassert>
#include <functional>
#include <iostream>
#include "handler.hpp"

int cap_covered_cnt(0);
int acer_covered_cnt(0);
int aceryang_covered_cnt(0);

struct uri_cap_handler
{
  void on_request(
    boost::system::error_code const& err,
    http::entity::request const & req,
    http::connection_ptr conn)
  {
    cap_covered_cnt++;
  }
};

struct uri_acer_handler
{
  void on_request(
    boost::system::error_code const& err,
    http::entity::request const & req,
    http::connection_ptr conn)
  {
    acer_covered_cnt++;
  }
};

struct uri_aceryang_handler
{
  void on_request(
    boost::system::error_code const& err,
    http::entity::request const & req,
    http::connection_ptr conn)
  {
    aceryang_covered_cnt++;
  }
};

void uri_acer(
    boost::system::error_code const& err,
    http::entity::request const & req,
    http::connection_ptr conn)
{
  acer_covered_cnt++;
}

int main()
{
  using namespace std::placeholders;

  http::uri_dispatcher uri_disp;
  uri_cap_handler cap_h;
  uri_acer_handler acer_h;
  uri_aceryang_handler aceryang_h;

  uri_disp.attach("cap", &uri_cap_handler::on_request, &cap_h);
  uri_disp.detach("cap", &uri_cap_handler::on_request, &cap_h);

  uri_disp["acer"].attach(
    &uri_acer_handler::on_request, &acer_h,
    _1, _2, _3); 

  uri_disp.attach("acer", &uri_acer);

  uri_disp["aceryang"].attach(
    &uri_aceryang_handler::on_request, &aceryang_h,
    _1, _2, _3); 

  boost::system::error_code ec;
  http::entity::request req;
  http::connection_ptr conn;

  uri_disp("acer/a").notify(ec,req,conn);
  uri_disp("acer/b").notify(ec,req,conn);
  uri_disp("aceryang/b").notify(ec,req,conn);

  try{
    uri_disp("benq/a").notify(ec,req,conn);
    assert(false && "Should throw");
  }catch(std::exception &e){
    std::cerr << e.what() << "\n";
  }

  assert(cap_covered_cnt == 0 && "Notification is failed");
  assert(acer_covered_cnt == 4 && "Notification is failed");
  assert(aceryang_covered_cnt == 1 && "Notification is failed");

  return 0;
}

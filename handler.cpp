#include "handler.hpp"

namespace http {

handler::handler(mode_t mode)
: mode_(mode), request_(0), response_(0)
{}

handler::~handler()
{
  connection_ptr_.reset();
}

void handler::on_response(
    boost::system::error_code const &err,
    http::entity::response const &response,
    http::connection_ptr conn)
{
  OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED;
  if(!err){
    connection_ptr_ = conn;
    response_ = &response;
  }else{
    handler_interface::on_response::notify(
      err, response, conn, MORE_DATA::NOMORE);
  }
}

void handler::on_request(
    boost::system::error_code const &err,
    http::entity::request const &request,
    http::connection_ptr conn)
{
  OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED;
  
  if(!err){
    connection_ptr_ = conn;
    request_ = &request;
  }else{
    handler_interface::on_request::notify(
      err, request, conn, MORE_DATA::NOMORE);
  }
}

handler::mode_t handler::mode() const
{ return mode_; }

connection_ptr handler::connection()
{ return connection_ptr_; }

entity::response const& handler::response() const
{ return *response_; }

entity::request const& handler::request() const
{ return *request_; }

void handler::notify(boost::system::error_code const &err)
{
  if(request_){
    handler_interface::on_request::notify(
      err, *request_, connection_ptr_, MORE_DATA::NOMORE);
  }else if(response_){
    handler_interface::on_response::notify(
      err, *response_, connection_ptr_, MORE_DATA::NOMORE);
  }else{
    assert("never reach here");
  }
}

} // namespace http

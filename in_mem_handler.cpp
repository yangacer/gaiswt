#include "in_mem_handler.hpp"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

namespace http {

in_memory_handler::in_memory_handler(mode_t mode)
: mode_(mode), request_(0), response_(0)
{}

in_memory_handler::~in_memory_handler()
{}

void in_memory_handler::on_response(
  boost::system::error_code const &err,
  http::entity::response const &response,
  http::connection_ptr conn)
{
  OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED;
    
  connection_ptr_ = conn;
  response_ = &response;
  start_transfer();
}

void in_memory_handler::on_request(
  boost::system::error_code const &err,
  http::entity::request const &request,
  http::connection_ptr conn)
{
  OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED;
  
  connection_ptr_ = conn;
  request_ = &request;
  start_transfer();
}

void in_memory_handler::start_transfer()
{
  using namespace boost::asio;

  // Start reading/writing remaining data until EOF.
  if(read == mode_){
    async_write(
      connection_ptr_->socket() , 
      connection_ptr_->io_buffer() ,
      transfer_at_least(1),
      boost::bind(
        &in_memory_handler::handle_transfer, 
        this,
        placeholders::error, 
        placeholders::bytes_transferred
        )
      );
  }else{
    // Start reading remaining data until EOF.
    async_read(
      connection_ptr_->socket() , 
      connection_ptr_->io_buffer() ,
      transfer_at_least(1),
      boost::bind(
        &in_memory_handler::handle_transfer, 
        this,
        placeholders::error, 0)
      );
  }
}

void in_memory_handler::handle_transfer(
  boost::system::error_code const &err,
  boost::uint32_t length)
{
  using namespace boost::asio;

  if(!err){
    if(read == mode_){
      connection_ptr_->io_buffer().consume(length);
      if(connection_ptr_->io_buffer().size())
        start_transfer();
    }else{
      start_transfer();
    }
  }else{
    notify(err);
  }
}

void in_memory_handler::notify(boost::system::error_code const &err)
{
  if(request_){
    interface::on_request::notify(
      err, *request_, connection_ptr_);
  }else if(response_){
    interface::on_response::notify(
      err, *response_, connection_ptr_);
  }else{
    assert("never reach here");
  }
}

} // namespace http

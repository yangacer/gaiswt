#include "in_mem_handler.hpp"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
//#include <iostream>

namespace http {

in_memory_handler::in_memory_handler(mode_t mode)
: handler(mode)
{}

in_memory_handler::~in_memory_handler()
{
  //std::cerr << "in_mem handler dtor\n";
}

void in_memory_handler::on_response(
  boost::system::error_code const &err,
  http::entity::response const &response,
  http::connection_ptr conn)
{
  OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED;
  
  handler::on_response(err, response, conn);
  if(!err)
    start_transfer();
}

void in_memory_handler::on_request(
  boost::system::error_code const &err,
  http::entity::request const &request,
  http::connection_ptr conn)
{
  OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED;
  
  handler::on_request(err, request, conn);
  if(!err)
    start_transfer();
}

void in_memory_handler::start_transfer()
{
  using namespace boost::asio;

  // Start reading/writing remaining data until EOF.
  if(read == mode()){
    async_write(
      connection()->socket() , 
      connection()->io_buffer() ,
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
      connection()->socket() , 
      connection()->io_buffer() ,
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
    if(read == mode()){
      connection()->io_buffer().consume(length);
      if(connection()->io_buffer().size())
        start_transfer();
    }else{
      start_transfer();
    }
  }else{
    notify(err);
  }
}


} // namespace http

#include "in_mem_handler.hpp"
#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace http {

in_memory_handler::in_memory_handler(
  mode_t mode, boost::uint32_t transfer_timeout_sec)
: handler(mode), transfer_timeout_(transfer_timeout_sec)
{}

in_memory_handler::~in_memory_handler()
{
  if(deadline_ptr_){
    deadline_ptr_->cancel();
    deadline_ptr_.reset();
  }
}

void in_memory_handler::on_response(
  boost::system::error_code const &err,
  http::entity::response const &response,
  http::connection_ptr conn)
{
  OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED;
  
  handler::on_response(err, response, conn);
  deadline_ptr_.reset(
    new boost::asio::deadline_timer(
      connection()->socket().get_io_service())); 
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
  deadline_ptr_.reset(
    new boost::asio::deadline_timer(
      connection()->socket().get_io_service())); 
  if(!err)
    start_transfer();
}

void in_memory_handler::start_transfer()
{
  using namespace boost::asio;

  // set timout
  deadline_ptr_->expires_from_now(
    boost::posix_time::seconds(transfer_timeout_));
  deadline_ptr_->async_wait(
    boost::bind(&in_memory_handler::handle_timeout, this));

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

void in_memory_handler::handle_timeout()
{}

} // namespace http

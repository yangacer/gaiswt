#include "in_mem_handler.hpp"
#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace http {

save_in_memory::save_in_memory(boost::asio::streambuf &buffer)
  : buffer_(buffer)
{}

void save_in_memory::on_response(
  http::entity::response const &response, 
  boost::asio::ip::tcp::socket &socket, 
  boost::asio::streambuf &front_data)
{
  OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED;

  using namespace boost::asio;
  
  socket_ = &socket;
  {
    std::ostream os(&buffer_);
    os << &front_data;
  }
  
  // Start reading remaining data until EOF.
  async_read(
    *socket_ , buffer_ ,
    transfer_at_least(1),
    boost::bind(
      &save_in_memory::handle_read, this,
      placeholders::error));
}

void save_in_memory::handle_read(boost::system::error_code err)
{
  using namespace boost::asio;

  if(!err){
    async_read(
      *socket_ , buffer_ ,
      transfer_at_least(1),
      boost::bind(
        &save_in_memory::handle_read, this,
        placeholders::error));

  }else if(err == boost::asio::error::eof){
    handler_interface::complete::notify();
  }else{
    handler_interface::error::notify(err);
  }
}

void save_in_memory::preprocess_error(boost::system::error_code err )
{
  handler_interface::error::notify(err);
}

} // namespace http

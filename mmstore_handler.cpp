#include "mmstore_handler.hpp"
#include <boost/asio.hpp>
#include <boost/bind.hpp>

namespace http {

save_to_mmstore::save_to_mmstore(mmstore &mms, std::string const& file)
  : mms_(mms), file_(file), region_(), offset_(0)
{}

save_to_mmstore::~save_to_mmstore(){}

void save_to_mmstore::on_response(
  http::entity::response const &response, 
  boost::asio::ip::tcp::socket &socket, 
  boost::asio::streambuf &front_data)
{
  OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED;

  socket_ = &socket;
  front_ = &front_data;
  
  mms_.async_get_region(
    region_, file_,
    mmstore::write, offset_,
    boost::bind(
      &save_to_mmstore::write_front,
      this,
      _1));
}

void save_to_mmstore::write_front(error_code err)
{
  if(!err){
    boost::asio::mutable_buffer dest(
      region_.buffer().first, 
      region_.buffer().second);

    boost::asio::const_buffer src(front_->data());

    boost::uint32_t cpy = boost::asio::buffer_copy(dest, src);
    //std::cout << "copied: " << cpy << "\n";
    offset_ += cpy;
    region_.commit(cpy);
    mms_.commit_region(region_, file_);

    mms_.async_get_region(
      region_, file_, 
      mmstore::write, offset_,
      boost::bind(
        &save_to_mmstore::handle_region,
        this,
        _1 ));
  }else{
    handler_interface::error::notify(err);
  }
}

void save_to_mmstore::handle_region(error_code err)
{
  if(!err){
    mmstore::region::raw_region_t buf = region_.buffer();
    socket_->async_receive(
      boost::asio::buffer(buf.first, buf.second),
      boost::bind(
        &save_to_mmstore::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred ));
  }else{
    handler_interface::error::notify(err);
  }
}

void save_to_mmstore::handle_read(error_code err, boost::uint32_t length)
{
  if(!err){
    region_.commit(length);
    offset_ += length;
    mms_.commit_region(region_, file_);
    mms_.async_get_region(
      region_, file_, 
      mmstore::write, offset_,
      boost::bind(
        &save_to_mmstore::handle_region,
        this, _1 ));
  }else if(err == boost::asio::error::eof){
    handler_interface::complete::notify();
  }else{
    handler_interface::error::notify(err);
  }

}

void save_to_mmstore::preprocess_error(boost::system::error_code err )
{
  handler_interface::error::notify(err);
}

} // namespace http

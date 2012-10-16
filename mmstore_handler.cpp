#include "mmstore_handler.hpp"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "entity.hpp"

#define N_BYTES_(N_KB) (N_KB << 10)

namespace http {

mmstore_handler::mmstore_handler(
  mmstore &mms, 
  std::string const& file, 
  mmstore::mode_t mode,
  boost::uint32_t max_n_kb_per_sec)
  : mms_(mms), mode_(mode),
  file_(file), region_(), offset_(0),
  stop_(false),
  max_n_kb_per_sec_(max_n_kb_per_sec),
  request_(0),
  response_(0)
{}

mmstore_handler::~mmstore_handler()
{
  //std::cout << "Average speed: " << 
  // (persist_speed_.average_speed()/(float)1024) << " KBps\n";
}

float mmstore_handler::speed_KBps() const
{
  return persist_speed_.average_speed() / (float)1024;
}

void mmstore_handler::start_get_region()
{
  if(stop_) return;

  mms_.async_get_region(
    region_, file_,
    mode_, offset_,
    boost::bind(
      &mmstore_handler::handle_region,
      this, _1)
    );
}

void mmstore_handler::on_response(
  boost::system::error_code const &err,
  http::entity::response const &response,
  http::connection_ptr conn)
{
  OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED;

  if(!err){
    connection_ptr_ = conn;
    response_ = &response;
    handler_interface::on_response::notify(
      err, response, conn, MORE_DATA::MORE);
    on_entity();
  }else{
    handler_interface::on_response::notify(
      err, response, conn, MORE_DATA::NOMORE);
  }
}

void mmstore_handler::on_request(
  boost::system::error_code const &err,
  http::entity::request const &request,
  http::connection_ptr conn)
{
  OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED;
  
  if(!err){
    connection_ptr_ = conn;
    request_ = &request;
    handler_interface::on_request::notify(
      err, request, conn, MORE_DATA::MORE);
    on_entity();
  }else{
    handler_interface::on_request::notify(
      err, request, conn, MORE_DATA::NOMORE);
  }
}

void mmstore_handler::on_entity()
{
  deadline_ptr_.reset(
    new boost::asio::deadline_timer(
      connection_ptr_->socket().get_io_service())); 

  persist_speed_.start_monitor();  

  if(mmstore::write == mode_){
    mms_.async_get_region(
      region_, file_,
      mmstore::write, offset_,
      boost::bind(
        &mmstore_handler::write_front,
        this,
        _1));
  }else{
    start_get_region();
  }
}

void mmstore_handler::write_front(error_code const &err)
{
  if(stop_) return;

  if(!err){
    boost::asio::mutable_buffer dest(
      region_.buffer().first, 
      region_.buffer().second);

    boost::asio::const_buffer src(connection_ptr_->io_buffer().data());

    boost::uint32_t cpy = boost::asio::buffer_copy(dest, src);

    persist_speed_.update_monitor(cpy);

    offset_ += cpy;
    region_.commit(cpy);
    mms_.commit_region(region_);
    
    start_get_region();
  }else{
    stop_ = true;
    notify(err);
  }
}

void mmstore_handler::handle_region(error_code const &err)
{
  using namespace boost::asio;
  if(stop_) return;
  
  if(!err){
    mmstore::region::raw_region_t buf = region_.buffer();
    per_transfer_speed_.start_monitor();
    if(mmstore::write == mode_){
      async_read(
        connection_ptr_->socket(),
        buffer(buf.first, buf.second),
        transfer_exactly(buf.second),
        boost::bind(
          &mmstore_handler::handle_transfer, this,
          placeholders::error,
          placeholders::bytes_transferred ));
    }else{
      async_write(
        connection_ptr_->socket(),
        buffer(buf.first, buf.second),
        transfer_exactly(buf.second),
        boost::bind(
          &mmstore_handler::handle_transfer, this,
          placeholders::error,
          placeholders::bytes_transferred ));
    }
  }else{
    stop_ = true;
    notify(err);
  }
}

void mmstore_handler::handle_transfer(error_code const &err, boost::uint32_t length)
{
  if(stop_) return;

  if(!err){
    persist_speed_.update_monitor(length);
    per_transfer_speed_.stop_monitor();
    if(mmstore::write == mode_)
      region_.commit(length);
    offset_ += length;
    mms_.commit_region(region_);

    boost::uint32_t delay = 
        (length / max_n_kb_per_sec_) - per_transfer_speed_.elapsed();

    if(!delay){
      start_get_region();
    }else{
      deadline_ptr_->expires_from_now(boost::posix_time::seconds(delay));
      deadline_ptr_->async_wait(
        boost::bind(&mmstore_handler::start_get_region, this));
    }
  }else{
    if(err == boost::asio::error::eof){
      persist_speed_.stop_monitor();
      deadline_ptr_->cancel();
    }
    stop_ = true;
    notify(err);
  }
}

void mmstore_handler::notify(error_code const &err)
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

#include "mmstore_handler.hpp"
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#define N_BYTES_(N_KB) (N_KB << 10)

namespace http {

save_to_mmstore::save_to_mmstore(
  mmstore &mms, std::string const& file, 
  boost::uint32_t max_n_kb_per_sec)
  : mms_(mms), 
  file_(file), region_(), offset_(0),
  stop_(false),
  max_n_kb_per_sec_(max_n_kb_per_sec)
{}

save_to_mmstore::~save_to_mmstore()
{
  std::cout << "Avg speed: " << (average_speed()/(float)1024) << " KBps\n";
}

void save_to_mmstore::start_receive()
{
  if(stop_) return;

  mmstore::region::raw_region_t buf = region_.buffer();
  agent_ptr_->socket().async_receive(
    boost::asio::buffer(buf.first, buf.second),
    boost::bind(
      &save_to_mmstore::handle_read, this,
      boost::asio::placeholders::error,
      boost::asio::placeholders::bytes_transferred ));
  
  // TODO Consider a slow connection that handle_read handler for a region
  // is triggered after deadline 
  boost::uint32_t period =
    mms_.maximum_region_size() / max_n_kb_per_sec_;
  
  deadline_ptr_->expires_from_now(boost::posix_time::seconds(period));
  deadline_ptr_->async_wait(
    boost::bind(&save_to_mmstore::start_get_region, this)
    );
}

void save_to_mmstore::start_get_region()
{
  if(stop_) return;

  mms_.async_get_region(
    region_, file_,
    mmstore::write, offset_,
    boost::bind(
      &save_to_mmstore::handle_region,
      this, _1)
    );
}

void save_to_mmstore::on_response(
  http::entity::response const &response,
  http::agent &agent_)
{
  OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED;

  agent_ptr_ = &agent_; 
  
  deadline_ptr_.reset(
    new boost::asio::deadline_timer(
      agent_ptr_->socket().get_io_service())); 
  
  start_monitor();  
  
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
  if(stop_) return;

  if(!err){
    boost::asio::mutable_buffer dest(
      region_.buffer().first, 
      region_.buffer().second);

    boost::asio::const_buffer src(agent_ptr_->front_data().data());

    boost::uint32_t cpy = boost::asio::buffer_copy(dest, src);

    update_monitor(cpy);

    offset_ += cpy;
    region_.commit(cpy);
    mms_.commit_region(region_, file_);
    
    start_get_region();
    /*
    mms_.async_get_region(
      region_, file_, 
      mmstore::write, offset_,
      boost::bind(
        &save_to_mmstore::handle_region,
        this,
        _1 ));
        */
  }else{
    stop_ = true;
    handler_interface::error::notify(err);
  }
}

void save_to_mmstore::handle_region(error_code err)
{
  if(stop_) return;
  
  if(!err){
    start_receive();
    /*
    mmstore::region::raw_region_t buf = region_.buffer();
    agent_ptr_->socket().async_receive(
      boost::asio::buffer(buf.first, buf.second),
      boost::bind(
        &save_to_mmstore::handle_read, this,
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred ));
        */
  }else{
    stop_ = true;
    handler_interface::error::notify(err);
  }
}

void save_to_mmstore::handle_read(error_code err, boost::uint32_t length)
{
  if(stop_) return;

  if(!err){
    update_monitor(length);
    region_.commit(length);
    offset_ += length;
    mms_.commit_region(region_, file_);
    /*
    mms_.async_get_region(
      region_, file_, 
      mmstore::write, offset_,
      boost::bind(
        &save_to_mmstore::handle_region,
        this, _1 ));
        */
  }else if(err == boost::asio::error::eof){
    stop_monitor();
    deadline_ptr_->cancel();
    stop_ = true;
    handler_interface::complete::notify();
  }else{
    stop_ = true;
    handler_interface::error::notify(err);
  }

}

void save_to_mmstore::preprocess_error(boost::system::error_code const &err )
{
  stop_ = true;
  handler_interface::error::notify(err);
}

} // namespace http

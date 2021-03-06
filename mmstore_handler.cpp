#include "mmstore_handler.hpp"
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include "entity.hpp"

#define N_BYTES_(N_KB) (N_KB << 10)

namespace http {

mmstore_handler::mmstore_handler(
  mmstore &mms, 
  std::string const& file, 
  handler::mode_t mode,
  boost::uint32_t transfer_timeout_sec,
  boost::uint32_t max_n_kb_per_sec)
  : handler(mode), 
  mms_(mms), 
  file_(file), region_(), offset_(0),
  stop_(false),
  transfer_timeout_(transfer_timeout_sec),
  max_n_kb_per_sec_(max_n_kb_per_sec)
{
  std::cerr << "mmstore_handler is constructed(" << 
    this << ")\n";
}

mmstore_handler::~mmstore_handler()
{
  if(deadline_ptr_){
    deadline_ptr_->cancel();
    deadline_ptr_.reset();
  }
  std::cerr << "mmstore_handler freed(" <<
    this <<   ")\n";
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
  
  mmstore::mode_t m = (handler::read == mode()) ?
    mmstore::read : mmstore::write
    ;

  mms_.async_get_region(
    region_, file_,
    m, offset_,
    boost::bind(
      &mmstore_handler::handle_region,
      shared_from_this(), _1)
    );

  std::cerr << "invoke async_get_region\n";
}

void mmstore_handler::on_response(
  boost::system::error_code const &err,
  http::entity::response const &response,
  http::connection_ptr conn)
{
  OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED;

  handler::on_response(err, response, conn);

  if(!err){
    handler_interface::on_response::notify(
      err, response, conn, MORE_DATA::MORE);
    on_entity();
  }
}

void mmstore_handler::on_request(
  boost::system::error_code const &err,
  http::entity::request const &request,
  http::connection_ptr conn)
{
  OBSERVER_TRACKING_OBSERVER_MEM_FN_INVOKED;
 
  handler::on_request(err, request, conn);

  if(!err){
    handler_interface::on_request::notify(
      err, request, conn, MORE_DATA::MORE);
    on_entity();
  }
}

void mmstore_handler::on_entity()
{
  std::cerr << "on_entity - connection status: " <<
    connection()->is_open() << "\n";

  deadline_ptr_.reset(
    new boost::asio::deadline_timer(
      connection()->socket().get_io_service())); 

  persist_speed_.start_monitor();  
  
  if(handler::write == mode()){
    mms_.async_get_region(
      region_, file_,
      mmstore::write, offset_,
      boost::bind(
        &mmstore_handler::write_front,
        shared_from_this(),
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

    boost::asio::const_buffer src(connection()->io_buffer().data());

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

  std::cerr << "handle region\n";

  if(stop_) return;
  
  if(!err){
    mmstore::region::raw_region_t buf = region_.buffer();
    per_transfer_speed_.start_monitor();
    
    //deadline_ptr_->expires_from_now(
    //  boost::posix_time::seconds(transfer_timeout_));
    //deadline_ptr_->async_wait(
    //  boost::bind(&mmstore_handler::handle_timeout, this));

    // TODO Use read_some / write_some instead of read/write exactly num
    // bytes
    std::cerr << "region size: " << buf.second << "\n";
    std::cerr << "connection status: " << connection()->is_open() << "\n";
    if(handler::write == mode()){
      async_read(
        connection()->socket(),
        buffer(buf.first, buf.second),
        transfer_exactly(buf.second),
        boost::bind(
          &mmstore_handler::handle_transfer, shared_from_this(),
          placeholders::error,
          placeholders::bytes_transferred ));
      std::cerr << "invoke async_read\n";
    }else{
      async_write(
        connection()->socket(),
        buffer(buf.first, buf.second),
        transfer_at_least(buf.second),
        boost::bind(
          &mmstore_handler::handle_transfer, shared_from_this(),
          placeholders::error,
          placeholders::bytes_transferred ));
      std::cerr << "invoke async_write to connection\n";
    }
  }else{
    std::cerr << "error: " << err.message() << "\n";
    stop_ = true;
    notify(err);
  }
}

void mmstore_handler::handle_transfer(error_code const &err, boost::uint32_t length)
{
  if(stop_) return;
  
  std::cerr << "handle_transfer\n";
  std::cerr << "connection status: " << connection()->is_open() << "\n";

  if(!err){
    persist_speed_.update_monitor(length);
    per_transfer_speed_.stop_monitor();
    if(handler::write == mode())
      region_.commit(length);
    offset_ += length;
    mms_.commit_region(region_);

    std::cerr << "actually transferred: " << length << "\n";

    boost::uint32_t delay = 
        (length / max_n_kb_per_sec_) - per_transfer_speed_.elapsed();

    if(!delay){
      start_get_region();
    }else{
      deadline_ptr_->expires_from_now(boost::posix_time::seconds(delay));
      deadline_ptr_->async_wait(
        boost::bind(&mmstore_handler::start_get_region,shared_from_this()));
    }
  }else{
    std::cerr << "error: " << err.message() << "\n";
    if(err == boost::asio::error::eof){
      persist_speed_.stop_monitor();
      deadline_ptr_->cancel();
    }
    stop_ = true;
    notify(err);
  }
}

void mmstore_handler::handle_timeout()
{
  if(stop_) return;
  stop_ = true;
  deadline_ptr_->cancel();
  notify(
    error_code(
      boost::system::errc::timed_out, 
      boost::system::system_category()));
}

} // namespace http

#include <exception>
#include <string>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include "mmstore.hpp"
#include "reply.hpp"

namespace asio = boost::asio;

struct deliver 
{
  typedef asio::ip::tcp tcp;
  typedef boost::system::error_code error_code;

  deliver(
    mmstore &mms,
    boost::shared_ptr<tcp::socket> socket,
    std::string const& vid, std::string const& format)
    : mms_(mms), socket_(socket), vid_name_(vid + format), offset_(0)
  {
  
  }

  void operator()()
  {
    namespace http = http::server4;
    using boost::bind;

    http::reply reply;
    std::stringstream header;
    boost::int64_t size;

    if(!mms_.is_in(vid_name_)){
      reply = http::reply::stock_reply(http::reply::not_found);
      goto ERROR_REPLY;
    }
    
    // write headers
    if(0 != (size = mms_.get_max_size(vid_name_))){
      header << "Content-Length:" << size;
    }
    
    asio::async_write(
        *socket_, header, 
        bind(&deliver::wait_region, this,
             asio::placeholders::bytes_transferred,
             asio::placeholders::error));
    
    return;

ERROR_REPLY:
    
    asio::async_write(
      *socket_, reply.to_buffers(), 
      bind(&deliver::finish, this, 0, asio::placeholders::error));
  }
  
  void wait_region(boost::uint32_t len = 0, error_code err = error_code())
  {
    using boost::bind;
    if(!err){
      mms_.async_get_region(
        region_, vid_name_,
        mmstore::read,
        offset_,
        bind(&deliver::handle_region, this, _1));
    }else{
      finish(err);
    }
  }

  void handle_region(error_code err)
  {
    using boost::bind;
    if(!err){
      mmstore::region::raw_region_t buf =
        region_.buffer();
      asio::async_write(
        *socket_, buf,
        bind(&deliver::region_sent, this,
             asio::placeholders::bytes_transferred,
             asio::placeholders::error));
    }else{
      finish(err);
    }
  }

  void region_sent(boost::uint32_t len, error_code err)
  {
    if(!err){
      offset_ += len;
      mms_.commit_region(region_, vid_name_);
      wait_region();
    }else{
      finish(err);
    }
  }

  void finish(size_t len, error_code err)
  {
    if(err)
      std::cerr << err.message() << "\n";
    socket_->shutdown(tcp::socket::shutdown_both, err);
  }

  mmstore &mms_;
  boost::shared_ptr<tcp::socket> socket_;
  std::string vid_name_;
  mmstore::region region_;
  boost::int64_t offset_;
};

int main(int argc, char** argv)
{
  try{
    
  }catch(std::exception &e){
    std::cerr << e.what();
  }
  return 0;
}

#include "agent2.hpp"
#include <iostream>
#include <exception>
#include <boost/shared_ptr.hpp>
#include "mmstore.hpp"
#include <boost/bind.hpp>
#include <functional>
#include <boost/enable_shared_from_this.hpp>
#include "observer/observable.hpp"

struct response_handler 
{
  response_handler(mmstore &mms)
    : mms_(mms), region_(), offset_(0)
  {}
  
  ~response_handler()
  { std::cerr << "response handler disposited\n"; }

  void on_ready(
    http::entity::response const &response, 
    boost::asio::ip::tcp::socket &socket, 
    boost::asio::streambuf &front_data)
  {
    socket_ = &socket;
    front_ = &front_data;
    //std::cout << "front data size: " << front_data.size() << "\n";
    
    mms_.async_get_region(
      region_, "response.tmp",
      mmstore::write, offset_,
      boost::bind(
        &response_handler::write_front,
        this,
        _1));
  }

  typedef boost::system::error_code error_code;
  
  void write_front(error_code err)
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
      mms_.commit_region(region_, "response.tmp");

      mms_.async_get_region(
        region_, "response.tmp", 
        mmstore::write, offset_,
        boost::bind(
          &response_handler::handle_region,
          this,
          _1 ));
    }else{
    
    }
  }

  void handle_region(error_code err)
  {
    if(!err){
      mmstore::region::raw_region_t buf = region_.buffer();
      socket_->async_receive(
        boost::asio::buffer(buf.first, buf.second),
        boost::bind(
          &response_handler::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred ));
    }
  }

  void handle_read(error_code err, boost::uint32_t length)
  {
    if(!err){
      region_.commit(length);
      offset_ += length;
      mms_.commit_region(region_, "response.tmp");
      mms_.async_get_region(
        region_, "response.tmp", 
        mmstore::write, offset_,
        boost::bind(
          &response_handler::handle_region,
          this, _1 ));
    }
  }

  void preprocess_error(boost::system::error_code err )
  {
    std::cout << err.message() << "\n"; 
  }

  mmstore &mms_;
  mmstore::region region_;
  boost::uint32_t offset_;
  boost::asio::ip::tcp::socket *socket_;
  boost::asio::streambuf *front_;
};


int main(int argc, char **argv)
{
  try
  {
    if (argc != 4)
    {
      std::cout << "Usage: agent2 <server> <service | port> <uri>\n";
      std::cout << "Example:\n";
      std::cout << "  agent www.boost.org 80 /LICENSE_1_0.txt\n";
      return 1;
    }
    
    namespace ph = std::placeholders;

    typedef http::entity::request request_t;

    boost::asio::io_service io_service;
    http::agent c(io_service);
    request_t request;

    mmstore mms("16384", "16");
    response_handler rep_handler(mms);

    mms.create("response.tmp");

    request = request_t::stock_request(request_t::GET_PAGE);
    request.query.path = argv[3];
    request.headers.emplace_back("Host", argv[1]);
    request.headers.emplace_back("User-Agent", "GAISWT/client");

    c.http::agent_interface::ready_for_read::attach_mem_fn(
      &response_handler::on_ready, &rep_handler, ph::_1, ph::_2, ph::_3);

    c.http::agent_interface::error::attach_mem_fn(
      &response_handler::preprocess_error, &rep_handler, ph::_1);

    c.run(argv[1], argv[2], request, "");
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cout << "Exception: " << e.what() << "\n";
  }
}

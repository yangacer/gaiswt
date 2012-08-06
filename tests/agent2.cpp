#include "agent2.hpp"
#include <iostream>
#include <exception>
#include <boost/shared_ptr.hpp>
#include "mmstore.hpp"
#include <boost/bind.hpp>
#include <functional>
#include <boost/enable_shared_from_this.hpp>
#include "observer/observable.hpp"



struct write_mmstore
: observer::observable<void(boost::uint32_t)>
{
  typedef observer::observable<void(boost::uint32_t)> on_written;

  write_mmstore(
    mmstore &mms, 
    std::string const &name, 
    http::asio::streambuf &streambuf)
    : mms_(mms), name_(name), 
    istream_(&streambuf)
  {
    
  }

  //async_write_mmstore() = default;

  typedef boost::system::error_code error_code;
  
  void async_write()
  {
    std::cout << "tellg:" << istream_.tellg() << "\n";

    mms_.async_get_region(
      region_, name_, mmstore::write, istream_.tellg(), 
      boost::bind(
        &write_mmstore::handle_written,
        this, _1));
  }
  
  void handle_written(error_code err)
  {
    if(err)
      notify(istream_.tellg());

    mmstore::region::raw_region_t buf =
      region_.buffer();
    istream_.read((char*)buf.first, buf.second);
    region_.commit(istream_.gcount());
    mms_.commit_region(region_, name_);

    if(istream_){
      async_write();    
    }else
      notify(istream_.tellg());
  }

  mmstore &mms_;
  std::string name_;
  mmstore::region region_;
  std::istream istream_;
};


struct response_handler 
{
  response_handler(mmstore &mms)
    : mms_(mms), region_(), offset_(0)//, wrt()
  {}
  
  ~response_handler()
  { std::cerr << "response handler disposited\n"; }

  void on_ready(
    http::entity::response const &response, 
    http::asio::ip::tcp::socket &socket, 
    http::asio::streambuf &front_data)
  {
    std::cout << "response ---- \n";
    std::cout << response;
    std::cout << "response ---- \n";

    socket_ = &socket;
    front_ = &front_data;
    std::cout << "front data size: " << front_data.size() << "\n";
    /*
    if(front_data.size()){
      wrt.reset(new write_mmstore(mms_, "response.tmp", front_data));
      wrt->on_written::attach_mem_fn(
        &response_handler::front_processed, 
        this, std::placeholders::_1);
      wrt->async_write();
    }else
      front_processed(0);
    */
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
      http::asio::mutable_buffer dest(
        region_.buffer().first, 
        region_.buffer().second);

      http::asio::const_buffer src(front_->data());

      boost::uint32_t cpy = http::asio::buffer_copy(dest, src);
      std::cout << "copied: " << cpy << "\n";
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
      // std::cout << "buffer size: " << buf.second << "\n";
      socket_->async_receive(
        http::asio::buffer(buf.first, buf.second),
        boost::bind(
          &response_handler::handle_read, this,
          http::asio::placeholders::error,
          http::asio::placeholders::bytes_transferred ));
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
  // boost::shared_ptr<write_mmstore> wrt;
  boost::uint32_t offset_;
  http::asio::ip::tcp::socket *socket_;
  http::asio::streambuf *front_;
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
    http::agent2 c(io_service);
    request_t request;

    mmstore mms("16384", "16");
    response_handler rep_handler(mms);

    mms.create("response.tmp");

    request = request_t::stock_request(request_t::GET_PAGE);
    request.uri = argv[3];
    request.headers.emplace_back("Host", argv[1]);

    std::cout << "request ------\n";
    std::cout << request;
    std::cout << "request ------\n";


    c.http::agent2_observable_interface::ready_for_read::attach_mem_fn(
      &response_handler::on_ready, &rep_handler, ph::_1, ph::_2, ph::_3);

    c.http::agent2_observable_interface::error::attach_mem_fn(
      &response_handler::preprocess_error, &rep_handler, ph::_1);

    c.run(argv[1], argv[2], request, "");
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cout << "Exception: " << e.what() << "\n";
  }
}

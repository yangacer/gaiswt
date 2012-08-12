#include "agent2.hpp"
#include <iostream>
#include <exception>
#include <boost/shared_ptr.hpp>
#include "mmstore.hpp"
#include <boost/bind.hpp>
#include <functional>
#include <boost/enable_shared_from_this.hpp>
#include "observer/observable.hpp"
#include <sstream>

namespace response_handler_interface {
  typedef observer::observable<void()> complete;
  typedef observer::observable<void(boost::system::error_code)> error;

  typedef observer::make_observable<
      observer::vector<
        complete, error
      >
    >::base concrete_interface;
}

namespace response_handler {

struct save_to_mmstore
: response_handler_interface::concrete_interface
{
  save_to_mmstore(mmstore &mms, std::string const& file)
    : mms_(mms), file_(file), region_(), offset_(0)
  {}
  
  ~save_to_mmstore(){}
  // { std::cerr << "response handler disposited\n"; }

  void on_ready(
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
      mms_.commit_region(region_, file_);

      mms_.async_get_region(
        region_, file_, 
        mmstore::write, offset_,
        boost::bind(
          &save_to_mmstore::handle_region,
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
          &save_to_mmstore::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred ));
    }
  }

  void handle_read(error_code err, boost::uint32_t length)
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
      response_handler_interface::complete::notify();
    }

  }

  void preprocess_error(boost::system::error_code err )
  {
    std::cout << err.message() << "\n"; 
  }

  OBSERVER_INSTALL_LOG_REQUIRED_INTERFACE_;

  mmstore &mms_;
  std::string file_;
  mmstore::region region_;
  boost::uint32_t offset_;
  boost::asio::ip::tcp::socket *socket_;
  boost::asio::streambuf *front_;
};

struct save_in_memory 
: response_handler_interface::concrete_interface
{
  void on_ready(
    http::entity::response const &response, 
    boost::asio::ip::tcp::socket &socket, 
    boost::asio::streambuf &front_data)
  {
    using namespace boost::asio;
    
    socket_ = &socket;
    iobuf_ = &front_data;

    // Start reading remaining data until EOF.
    async_read(*socket_ , *iobuf_ ,
               transfer_at_least(1),
               boost::bind(
                 &save_in_memory::handle_read, this,
                 placeholders::error));
  }

  void handle_read(boost::system::error_code err)
  {
    using namespace boost::asio;

    if(!err){
      
      async_read(*socket_ , *iobuf_ ,
                 transfer_at_least(1),
                 boost::bind(
                   &save_in_memory::handle_read, this,
                   placeholders::error));

    }else if(err == boost::asio::error::eof){
      
    }else{
      
    }
  }

  void preprocess_error(boost::system::error_code err )
  {
   
  }

  boost::asio::ip::tcp::socket *socket_;
  boost::asio::streambuf *iobuf_;
};

template<typename Handler>
void hook_helper(http::agent &agent, Handler &handler)
{
  namespace ph = std::placeholders;

  agent.http::agent_interface::ready_for_read::attach_mem_fn(
    &Handler::on_ready, &handler, ph::_1, ph::_2, ph::_3);

  agent.http::agent_interface::error::attach_mem_fn(
    &Handler::preprocess_error, &handler, ph::_1);
}

} // namespace response_handler

void on_complete()
{
  OBSERVER_TRACKING_OBSERVER_FN_INVOKED;
}

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
    http::agent agent(io_service);
    request_t request;

    mmstore mms("16384", "16");
    response_handler::save_to_mmstore handler(mms, "response.tmp");
    handler.response_handler_interface::complete::attach(&on_complete);

    // Create file in mmstore.
    mms.create("response.tmp");
    
    // Setup request
    request = request_t::stock_request(request_t::GET_PAGE);
    request.query.path = argv[3];
    request.headers.emplace_back("Host", argv[1]);
    request.headers.emplace_back("User-Agent", "GAISWT/client");

    // Connect handler and agent
    response_handler::hook_helper(agent, handler);

    // Setup log
    std::stringstream log;
    logger::singleton().set(log);

    agent.run(argv[1], argv[2], request, "");
    io_service.run();
    std::cout << "LOG---\n" << log.str();
  }
  catch (std::exception& e)
  {
    std::cout << "Exception: " << e.what() << "\n";
  }
}

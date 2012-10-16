#include "agent.hpp"
#include <iostream>
#include <exception>
#include <boost/shared_ptr.hpp>
#include "mmstore.hpp"
#include <boost/bind.hpp>
#include <functional>
#include <boost/enable_shared_from_this.hpp>
#include "observer/observable.hpp"
#include <sstream>
#include <signal.h>
#include <boost/asio/signal_set.hpp>
#include "connection_manager.hpp"
#include "mmstore_handler.hpp"
#include "in_mem_handler.hpp"

void on_complete(
  boost::system::error_code const &err,
  http::entity::response const &response,
  http::connection_ptr conn)
{
  OBSERVER_TRACKING_OBSERVER_FN_INVOKED;

  if(err == boost::asio::error::eof || !err)
    std::cout << "---- handler is completed ---- \n";
  else
    std::cout << "Error: " << err.message() << "\n";

#ifdef OBSERVER_ENABLE_TRACKING
  std::cout << "LOG---\n" <<
    logger::singleton().get().rdbuf();
#endif
}

void on_mem_complete(
    boost::system::error_code const &err,
    http::entity::response const &response,
    http::connection_ptr conn)
{
  OBSERVER_TRACKING_OBSERVER_FN_INVOKED;

  if(err == boost::asio::error::eof || !err){
    std::cout << "Received size: " << conn->io_buffer().size() << "\n";
    std::cout << "---- handler is completed ---- \n";
  }else{
    std::cout << "Error: " << err.message() << "\n";
  }
}

int main(int argc, char **argv)
{
  try
  {
    if (argc != 4)
    {
      std::cout << "Usage: agent <server> <service | port> <uri>\n";
      std::cout << "Example:\n";
      std::cout << "  agent www.boost.org 80 /LICENSE_1_0.txt\n";
      return 1;
    }
    
    namespace ph = std::placeholders;

    typedef http::entity::request request_t;
    
    boost::asio::io_service io_service;
    http::connection_manager connection_manager;

    http::agent 
      agent_1(io_service, connection_manager), 
      agent_2(io_service, connection_manager);
    request_t request;

    mmstore mms(io_service, "1048576", "16");
    
    http::mmstore_handler mm_handler(mms, "response.tmp", mmstore::write);
    http::in_memory_handler mem_handler(http::in_memory_handler::write);

    mm_handler.http::handler_interface::on_response::attach(
      &on_complete, ph::_1, ph::_2, ph::_3);

    mem_handler.http::handler_interface::on_response::attach(
      &on_mem_complete, ph::_1, ph::_2, ph::_3);

    // Create file in mmstore.
    mms.create("response.tmp");
    
    // Setup request
    request = request_t::stock_request(request_t::GET_PAGE);
    request.query.path = argv[3];
    request.headers.emplace_back("Host", argv[1]);
    request.headers.emplace_back("User-Agent", "GAISWT/client");
    

#ifdef OBSERVER_ENABLE_TRACKING
    // Setup log
    std::stringstream log;
    logger::singleton().set(log);
#endif
    
    boost::asio::signal_set signals(io_service);
    signals.add(SIGINT);
    signals.add(SIGTERM);
#if defined(SIGQUIT)
    signals.add(SIGQUIT);
#endif // defined(SIGQUIT)
    signals.async_wait(boost::bind(
          &boost::asio::io_service::stop, &io_service));
    signals.async_wait(boost::bind(
          &boost::asio::io_service::stop, &mms.get_io_service()));

    agent_1.run(argv[1], argv[2], request).on_response(mm_handler);
    agent_2.run(argv[1], argv[2], request).on_response(mem_handler);

    io_service.run();

#ifdef OBSERVER_ENABLE_TRACKING
    std::cout << "LOG---\n" << log.str();
#endif
  }
  catch (std::exception& e)
  {
    std::cout << "Exception: " << e.what() << "\n";
  }
}

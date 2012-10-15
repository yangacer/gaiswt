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

void on_complete()
{
  OBSERVER_TRACKING_OBSERVER_FN_INVOKED;
  std::cout << "---- handler is completed ---- \n";
#ifdef OBSERVER_ENABLE_TRACKING
  std::cout << "LOG---\n" <<
    logger::singleton().get().rdbuf();
#endif
}

void on_error(boost::system::error_code const &err)
{
  OBSERVER_TRACKING_OBSERVER_FN_INVOKED;
  std::cout << "Error: " << err.message() << "\n";
}

void on_mem_complete(boost::asio::streambuf &result)
{
  OBSERVER_TRACKING_OBSERVER_FN_INVOKED;
  //std::cout << &result;
  std::cout << "Received size: " << result.size() << "\n";
  std::cout << "---- handler is completed ---- \n";
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
    boost::asio::signal_set signals(io_service);
    http::connection_manager connection_manager;

    http::agent 
      agent_1(io_service, connection_manager), 
      agent_2(io_service, connection_manager);
    request_t request;

    mmstore mms(io_service, "1048576", "16");
    boost::asio::streambuf result;
    
    http::mmstore_handler mm_handler(mms, "response.tmp", mmstore::write);
    http::save_in_memory mem_handler(result);

    mm_handler.http::handler_interface::complete::attach(&on_complete);
    mm_handler.http::handler_interface::error::attach(&on_error, ph::_1);

    mem_handler.http::handler_interface::complete::attach(
      &on_mem_complete, std::ref(result));

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

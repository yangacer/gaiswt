#include "peer_manager.hpp"
#include <iostream>
#include "coroutine.hpp"


struct agent : coroutine
{
  typedef http::asio::ip::tcp tcp;
  typedef boost::system::error_code error_code;

  agent(boost::shared_ptr<tcp::socket> socket)
  : socket_(socket)
  {}

#include "yield.hpp"
  
  void operator()(error_code err = error_code())
  {
    reenter(this){
     //  yield async_read | async_write
    } // reenter
  }

#include "unyield.hpp"
  
  boost::shared_ptr<tcp::socket> socket_;
};

struct plan 
: coroutine
                
{
  typedef http::asio::ip::tcp tcp;
  typedef boost::system::error_code error_code;

  plan(http::peer_manager & peer_manager,

       )
  : peer_manager_(peer_manager)
  {}

  void operator()(error_code err = error_code())
  {
    
  }

  http::peer_manager peer_manager_;
};


int main(int argc, char** argv)
{
  try{
    if(argc != 3){
      std::cout << "Usage: peer_manager <server> <service|port>\n";
      return 1;
    }

    typedef http::asio::ip::tcp tcp;

    http::asio::io_service io_service;
    http::peer_manager pm(io_service);

    boost::shared_ptr<tcp::socket> s = pm.async_create_socket(server, service);

    io_service.run();
  }catch(std::exception &e){
    std::cout << "Exception: " << e.what() << "\n";
  }

}

#include "connector.hpp"
#include <boost/bind.hpp>
#include <iostream>
#include <exception>

struct conn_handler 
{
  conn_handler(boost::asio::io_service &io_service)
  : socket(new boost::asio::ip::tcp::socket(io_service))
  {}

  conn_handler(conn_handler const &cp)
  : socket(cp.socket)
  {
    std::cout<<"conn_handler copied!\n";
  }
  
  void operator()(boost::system::error_code err, size_t length=0)
  {
    if(!err){
      if(!socket->is_open()) return;
      std::cout << "Connected!\n";
      
      
      std::cout << "Socket properties:\n";
      {
        http::asio::socket_base::keep_alive option;
        socket->get_option(option);
        bool is_set = option.value();
        if(is_set)
          std::cout << "keep-alive\n";
        else
          std::cout << "close\n";
      }
      if(socket->non_blocking())
        std::cout << "nonblocking\n";
      else
        std::cout << "blocking\n";
      {
        http::asio::socket_base::reuse_address option;
        socket->get_option(option);
        if(option.value())
          std::cout << "reuseaddr\n";
        else
          std::cout << "non-reuseaddr\n";
      }
    }else{
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  boost::shared_ptr<boost::asio::ip::tcp::socket> socket;
};

int main(int argc, char** argv)
{
  try{
    if(argc != 3){
      std::cout << "Usage: connector <server> <service|port>\n";
      return 1;
    }

    http::asio::io_service io_service;
    http::asio::ip::tcp::resolver resolver(io_service);

    http::connector c(io_service, resolver);

    conn_handler ch(io_service);
    /// XXX Here is a problem. If we want to prevent copying handler
    //  everywhere then we have to wrap it in a way as below.
    
    c.async_resolve_and_connect(
      *ch.socket, argv[1], argv[2],
      boost::bind(
        &conn_handler::operator(), 
        &ch, http::asio::placeholders::error, 0
        )
      );
    
    // XXX Version of allowing copy
    // Recommend to use this version if all members of the handler are
    // warpped by shared_ptr< >.
    // c.async_resolve_and_connect(*ch.socket, argv[1], argv[2], ch);

    io_service.run();
  } catch (std::exception& e){
    std::cout << "Exception: " << e.what() << "\n";
  }
  return 0;
}

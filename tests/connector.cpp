#include "connector.hpp"

struct conn_handler 
{
  typedef void result_type;

  conn_handler(boost::asio::io_service &io_service)
  : socket(new boost::asio::ip::tcp::socket(io_service))
  {}

  void operator()(boost::system::error_code err, size_t length=0)
  {
    if(!err){
      if(socket->is_open())
        std::cout << "Connected!\n";
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
    
    c.async_resolve_and_connect(*ch.socket, argv[1], argv[2], ch);

    io_service.run();
  } catch (std::exception& e){
    std::cout << "Exception: " << e.what() << "\n";
  }
  return 0;
}

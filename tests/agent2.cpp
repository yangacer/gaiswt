#include "agent2.hpp"
#include <iostream>
#include <exception>

struct response_handler
{
  response_handler(mmstore &mms)
    : mms_(mms)
  {}

  bool on_before_read(mmstore::region &region)
  {
    
  }

  bool on_after_read(mmstore::region &region)
  {
    
  }

  void on_error(boost::system::error_code const &err)
  {
    
  }

  mmstore &mms_;
};

int main(int argc, char **argv)
{
  try
  {
    if (argc != 4)
    {
      std::cout << "Usage: async_agent <server> <service | port> <path>\n";
      std::cout << "Example:\n";
      std::cout << "  async_agent www.boost.org 80 /LICENSE_1_0.txt\n";
      return 1;
    }

    boost::asio::io_service io_service;
    http::agent2 c(io_service);
    http::entity::header header;
    
    header.method = "GET";
    header.http_version_major = header.http_version_minor = 1;
    header.uri = "/";

    c.run(argv[1], argv[2], header, "");
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cout << "Exception: " << e.what() << "\n";
  }
}

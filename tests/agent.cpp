#include "agent.hpp"
#include <iostream>
#include <exception>

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
    http::agent c(io_service, argv[1], argv[2], argv[3]);
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cout << "Exception: " << e.what() << "\n";
  }
}

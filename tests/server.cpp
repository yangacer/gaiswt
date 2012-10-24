#include <iostream>
#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include "server.hpp"
#include "connection_manager.hpp"

int main(int argc, char* argv[])
{
  try
  {
    // Check command line arguments.
    if (argc != 4)
    {
      std::cerr << "Usage: http_server <address> <port> <document_root>\n";
      std::cerr << "  For IPv4, try:\n";
      std::cerr << "    receiver 0.0.0.0 80 \n";
      std::cerr << "  For IPv6, try:\n";
      std::cerr << "    receiver 0::0 80 \n";
      return 1;
    }

    boost::asio::io_service io_service;
    mmstore mms(io_service, "100000000", "100", "server.mms");
    http::connection_manager connection_manager;
    http::server server(io_service, connection_manager, mms, argv[3]);
    server.run(argv[1], argv[2]);

    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cerr << "exception: " << e.what() << "\n";
  }

  return 0;
}

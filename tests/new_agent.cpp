#include "agent.hpp"
#include "mmstore_handler.hpp"
#include "in_mem_handler.hpp"
#include <boost/asio.hpp>

int main()
{
  boost::asio::io_service io_service;
  http::connection_manager connection_manager;
  boost::asio::streambuf body;
  mmstore mms("10240", "2");

  http::agent agent(io_service, connection_manager);

  http::save_to_mmstore reponse_handler(mms, "upload.file");
  http::load_from_memory request_handler(body);

  agent.run(server, service, request)
    .on_request(
        boost::bind(
          &http::load_from_memory::handle_request, 
          this, _1, _2, _3))
    .on_response( 
        boost::bind(
          &http::save_to_mmstore::handle_response, 
          this, _1, _2, _3));

  io_service.run();
  return 0;
}

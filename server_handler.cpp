#include "server_handler.hpp"
#include <iostream>

namespace http {

server_handler::server_handler(mmstore &mms, std::string const& document_root)
: mms_(mms), document_root_(document_root)
{}

void server_handler::on_request(
    boost::system::error_code const &err,
    http::entity::request const &request,
    http::connection_ptr conn)
{
  std::string request_path(request.query.path);


  // Request path must be absolute and not contain "..".
  if (request_path.empty() || request_path[0] != '/'
      || request_path.find("..") != std::string::npos)
  {
    // TODO 
    //rep = response::stock_response(response::bad_request);
    //return;
  }

  // If path ends in slash (i.e. is a directory) then add "index.html".
  if (request_path[request_path.size() - 1] == '/')
    request_path += "index.html";

  // Determine the file extension.
  std::size_t last_slash_pos = request_path.find_last_of("/");
  std::size_t last_dot_pos = request_path.find_last_of(".");
  std::string extension;

  if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
    extension = request_path.substr(last_dot_pos + 1);

  // Open the file to send back.
  std::string full_path = document_root_ + request_path;

  
}

} // namespace http

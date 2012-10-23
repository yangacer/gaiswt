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
  // TODO Impl
}

} // namespace http

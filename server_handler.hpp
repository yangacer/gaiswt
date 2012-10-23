#ifndef GAISWT_SERVER_HANDLER_HPP_
#define GAISWT_SERVER_HANDLER_HPP_
#include "mmstore_handler.hpp"
#include <boost/shared_ptr.hpp>

namespace http {

struct server_handler
{
  server_handler(mmstore &mms, std::string const& document_root);
  
  void on_request(
    boost::system::error_code const &err,
    http::entity::request const &request,
    http::connection_ptr conn);

private:
  mmstore &mms_;
  std::string document_root_;
  boost::shared_ptr<mmstore_handler> handler_;
};

} // namespace http

#endif // header guard


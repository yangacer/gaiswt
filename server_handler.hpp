#ifndef GAISWT_SERVER_HANDLER_HPP_
#define GAISWT_SERVER_HANDLER_HPP_
#include "mmstore_handler.hpp"
#include "in_mem_handler.hpp"
#include <boost/shared_ptr.hpp>

namespace http {

struct server_handler
  : handler
{
  server_handler(mmstore &mms, std::string const& document_root);
  ~server_handler();

  void on_request(
    boost::system::error_code const &err,
    http::entity::request const &request,
    http::connection_ptr conn);

protected:

  void send_header();
  
  void handle_send_header(
    boost::system::error_code const &err,
    boost::uint32_t length);

private:
  typedef entity::response response;

  mmstore &mms_;
  std::string document_root_;
  entity::response resp_;
  boost::shared_ptr<mmstore_handler> mms_handler_;
  boost::shared_ptr<in_memory_handler> in_mem_handler_;
};

} // namespace http

#endif // header guard


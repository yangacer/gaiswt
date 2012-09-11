#ifndef GAISWT_MMSTORE_HANLDER_HPP_
#define GAISWT_MMSTORE_HANDLER_HPP_

#include <string>
#include <boost/cstdint.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>

#include "mmstore.hpp"
#include "entity.hpp"
#include "handler.hpp"

namespace http{

class save_to_mmstore
: public handler_interface::concrete_interface
{
public:

  OBSERVER_INSTALL_LOG_REQUIRED_INTERFACE_;

  save_to_mmstore(mmstore &mms, std::string const& file, 
                  boost::uint32_t max_n_kb_per_sec = 4096);
  
  ~save_to_mmstore();

  void on_response(
    http::entity::response const &response, 
    boost::asio::ip::tcp::socket &socket, 
    boost::asio::streambuf &front_data);

  void preprocess_error(boost::system::error_code err );

protected:

  typedef boost::system::error_code error_code;
  
  void write_front(error_code err);
  
  void handle_region(error_code err);

  void handle_read(error_code err, boost::uint32_t length);

private:

  mmstore &mms_;
  std::string file_;
  mmstore::region region_;
  boost::uint32_t offset_;
  boost::asio::ip::tcp::socket *socket_;
  boost::asio::streambuf *front_;
  boost::uint32_t max_n_kb_per_sec_;
}; 

} // namespace http

#endif // header guard

#ifndef GAISWT_MMSTORE_HANLDER_HPP_
#define GAISWT_MMSTORE_HANDLER_HPP_

#include <string>
#include <boost/cstdint.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/shared_ptr.hpp>

#include "mmstore.hpp"
#include "entity.hpp"
#include "interface.hpp"
#include "speed_monitor.hpp"
#include "connection.hpp"

namespace http{

// TODO Timeout/minimum speed 
class mmstore_handler
: public interface::concrete_interface
{
public:

  OBSERVER_INSTALL_LOG_REQUIRED_INTERFACE_;

  mmstore_handler(
    mmstore &mms,
    std::string const& file, 
    mmstore::mode_t mode,
    boost::uint32_t max_n_kb_per_sec = 4096);
  
  virtual ~mmstore_handler();

  void on_response(
    boost::system::error_code const &err,
    http::entity::response const &response,
    http::connection_ptr conn);

  void on_request(
    boost::system::error_code const &err,
    http::entity::request const &request,
    http::connection_ptr conn);
  
  float speed_KBps() const;

protected:

  void on_entity();

  typedef boost::system::error_code error_code;
  
  void write_front(error_code const &err);
  
  void handle_region(error_code const &err);

  void handle_transfer(error_code const &err, boost::uint32_t length);
  
  void start_get_region();

  void notify(error_code const& err);

private:

  mmstore &mms_;
  std::string file_;
  mmstore::mode_t mode_;
  mmstore::region region_;
  boost::uint32_t offset_;
  http::connection_ptr connection_ptr_;
  boost::shared_ptr<boost::asio::deadline_timer> deadline_ptr_;
  bool stop_;
  boost::uint32_t max_n_kb_per_sec_;
  speed_monitor persist_speed_, per_transfer_speed_;

  entity::request const *request_;
  entity::response const *response_;
}; 


} // namespace http

#endif // header guard

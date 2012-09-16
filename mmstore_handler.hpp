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
#include "handler.hpp"
#include "speed_monitor.hpp"

namespace http{

class save_to_mmstore
: public handler_interface::concrete_interface
{
public:

  OBSERVER_INSTALL_LOG_REQUIRED_INTERFACE_;

  save_to_mmstore(mmstore &mms, std::string const& file, 
                  boost::uint32_t max_n_kb_per_sec = 4096);
  
  virtual ~save_to_mmstore();

  void on_response(
    http::entity::response const &response,
    http::agent &agent_);

  void preprocess_error(boost::system::error_code const &err );

protected:

  typedef boost::system::error_code error_code;
  
  void write_front(error_code err);
  
  void handle_region(error_code err);

  void handle_read(error_code err, boost::uint32_t length);
  
  void start_get_region();

  //void start_receive();
private:

  mmstore &mms_;
  std::string file_;
  mmstore::region region_;
  boost::uint32_t offset_;
  //boost::asio::ip::tcp::socket *socket_;
  //boost::asio::streambuf *front_;
  http::agent* agent_ptr_;
  boost::shared_ptr<boost::asio::deadline_timer> deadline_ptr_;
  bool stop_;
  boost::uint32_t max_n_kb_per_sec_;
  speed_monitor persist_speed_, per_transfer_speed_;
}; 

} // namespace http

#endif // header guard

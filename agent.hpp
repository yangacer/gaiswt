#ifndef GAISWT_AGENT2_HPP_
#define GAISWT_AGENT2_HPP_

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <string>
#include "entity.hpp"
#include "observer/observable.hpp"
#include "connection.hpp"

namespace http {

class agent;
class connection_manager;

namespace parser{
  template<typename T> struct response_first_line;
  template<typename T> struct header_list;
} // namespace parser

namespace agent_interface {

  typedef observer::observable<
    void(entity::response const&, 
         http::connection_ptr)> ready_for_read;

  typedef observer::observable<
    void(boost::system::error_code const&)> error;

  typedef observer::make_observable<
      observer::vector<
        ready_for_read,
        error
      >
    >::base concrete_interface;
}

class agent
  : public agent_interface::concrete_interface
{
  typedef boost::asio::ip::tcp tcp;
public:

  agent(
    boost::asio::io_service& io_service, 
    connection_manager &cm);
  
  virtual ~agent();

  boost::asio::streambuf &front_data();
  
  tcp::socket &socket();

  void run(std::string const &server, 
           std::string const &service, 
           entity::request const &request,
           std::string const &body);

  //void cancel();
  
  //void finish();

protected:
  
  void handle_resolve(
    const boost::system::error_code& err,
    tcp::resolver::iterator endpoint_iterator);
  
  void handle_connect(
    boost::system::error_code const &err,
    tcp::resolver::iterator endpoint_interator);

  void handle_write_request(
    boost::system::error_code const &err, 
    boost::uint32_t len);
  
  void handle_read_status_line(boost::system::error_code const &err);

  void handle_read_headers(boost::system::error_code const &err);
  
  void redirect();

  void check_deadline();

  OBSERVER_INSTALL_LOG_REQUIRED_INTERFACE_

private:

  typedef boost::asio::buffers_iterator<
    boost::asio::streambuf::const_buffers_type
    > 
    buffer_iterator_t;

  tcp::resolver resolver_;
  http::connection_ptr connection_ptr_;
  entity::response response_;
  entity::request request_;
  int redirect_count_;
  boost::asio::deadline_timer deadline_;
  bool stop_;

};

} // namespace http

#endif // header guard

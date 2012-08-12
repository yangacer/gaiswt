#ifndef GAISWT_AGENT2_HPP_
#define GAISWT_AGENT2_HPP_

#include <boost/asio.hpp>
#include "entity.hpp"
#define OBSERVER_ENABLE_TRACKING
#include "observer/observable.hpp"

namespace http {

class agent;

namespace parser{
  template<typename T> struct response_first_line;
  template<typename T> struct header_list;
} // namespace parser

namespace agent_interface {

  typedef observer::observable<
    void(entity::response const&, 
         boost::asio::ip::tcp::socket &, 
         boost::asio::streambuf &)
    > ready_for_read;

  typedef observer::observable<void(boost::system::error_code)> error;

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

  agent(boost::asio::io_service& io_service);
  
  virtual ~agent();

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
  
  void handle_connect(boost::system::error_code const &err);

  void handle_write_request(
    boost::system::error_code const &err, 
    boost::uint32_t len);
  
  void handle_read_status_line(boost::system::error_code const &err);

  void handle_read_headers(boost::system::error_code const &err);
  
  void redirect();
  // void handle_read_content(boost::system::error_code const &err);

  OBSERVER_INSTALL_LOG_REQUIRED_INTERFACE_

private:

  typedef boost::asio::buffers_iterator<
    boost::asio::streambuf::const_buffers_type
    > 
    buffer_iterator_t;

  tcp::resolver resolver_;
  tcp::socket socket_;
  boost::asio::streambuf iobuf_;
  entity::response response_;
  entity::request request_;
  int redirect_count_;
  // bool is_canceled_;
};

} // namespace http

#endif // header guard

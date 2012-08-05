#ifndef GAISWT_AGENT2_HPP_
#define GAISWT_AGENT2_HPP_

#include <boost/asio.hpp>
#include "entity.hpp"
#include "observer/observable.hpp"
//#include "mmstore.hpp"

namespace http {

namespace asio = boost::asio;
namespace obs = observer;

class agent2;

namespace agent2_observable_interface {
  // type tags
  //struct tag_before_read;
  //struct tag_after_read;
 
  // interfaces
  //typedef obs::observable<bool(mmstore::region &region, agent2 &), tag_before_read> before_read;
  //typedef obs::observable<bool(mmstore::region &region, agent2 &), tag_after_read> after_read;
  typedef obs::observable<
    void(entity::response const&, asio::ip::tcp::socket &, asio::streambuf &)
    > ready_for_read;

  typedef obs::observable<void(boost::system::error_code)> error;

  typedef obs::make_observable<
      obs::vector<
        ready_for_read,
        error
      >
    >::base interface;
}

class agent2
  : public agent2_observable_interface::interface
{
  typedef asio::ip::tcp tcp;
public:
  typedef agent2_observable_interface::ready_for_read read_for_read;
  typedef agent2_observable_interface::error error;

  agent2(boost::asio::io_service& io_service);
  
  virtual ~agent2();

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
  
  // void handle_read_content(boost::system::error_code const &err);

private:
  //mmstore::region region_;
  tcp::resolver resolver_;
  tcp::socket socket_;
  asio::streambuf iobuf_;
  entity::response response_;
  entity::request request_;
  // bool is_canceled_;
};

}

#endif // header guard

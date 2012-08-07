#include "parser.hpp"

#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include "agent2.hpp"
#include "utility.hpp"

namespace http {
 
agent2::agent2(asio::io_service& io_service)
  : resolver_(io_service),
    socket_(io_service)
{}

agent2::~agent2()
{}

void agent2::run(std::string const &server, 
         std::string const &service, 
         entity::request const &request,
         std::string const &body)
{
  // store for redirection
  request_ = request;

  std::ostream request_stream(&iobuf_);
  request_stream.flush();
  request_stream << request;

  tcp::resolver::query query(server, service);
  resolver_.async_resolve(query,
      boost::bind(&agent2::handle_resolve, this,
        asio::placeholders::error,
        asio::placeholders::iterator));
}


void agent2::handle_resolve(
  const boost::system::error_code& err,
  tcp::resolver::iterator endpoint_iterator)
{
  if (!err){
    asio::async_connect(
      socket_, endpoint_iterator,
      boost::bind(&agent2::handle_connect, this,
                  asio::placeholders::error));
  }else{
    agent2_observable_interface::error::notify(err);
  }
}

void agent2::handle_connect(const boost::system::error_code& err)
{
  if (!err){
    asio::async_write(socket_, iobuf_,
        boost::bind(&agent2::handle_write_request, this,
          asio::placeholders::error,
          asio::placeholders::bytes_transferred
          ));
  }else{
    agent2_observable_interface::error::notify(err);
  }
}

void agent2::handle_write_request(
  boost::system::error_code const & err,
  boost::uint32_t len)
{
  if (!err){
    iobuf_.consume(len);
    asio::async_read_until(
      socket_, iobuf_, "\r\n",
      boost::bind(&agent2::handle_read_status_line, this,
                  asio::placeholders::error));
  }else{
    agent2_observable_interface::error::notify(err);
  }
}

void agent2::handle_read_status_line(const boost::system::error_code& err)
{
  namespace sys = boost::system;

  using boost::lexical_cast;

  if (!err) {
    // Check that response is OK.
    auto beg(asio::buffers_begin(iobuf_.data())), 
         end(asio::buffers_end(iobuf_.data()));
    //std::cout << "First line handler has size: " << end - beg << "\n";
    
    if(!parser::parse_response_first_line(beg, end, response_)){
      agent2_observable_interface::error::notify(
        sys::error_code(
          sys::errc::bad_message,
          sys::system_category()));
      return;
    }
    
    iobuf_.consume(beg - asio::buffers_begin(iobuf_.data()));

    // Read the response headers, which are terminated by a blank line.
    asio::async_read_until(socket_, iobuf_, "\r\n\r\n",
        boost::bind(&agent2::handle_read_headers, this,
          asio::placeholders::error));
  } else {
    agent2_observable_interface::error::notify(err);
  }
}

void agent2::handle_read_headers(const boost::system::error_code& err)
{
  namespace sys = boost::system;
  // typedef parser::istream_iterator iter_t;

  if (!err) {
    // Process the response headers.
    auto beg(asio::buffers_begin(iobuf_.data())), 
         end(asio::buffers_end(iobuf_.data()));

    if(!parser::parse_header_list(beg, end, response_.headers))
      goto BAD_MESSAGE;

    iobuf_.consume(beg - asio::buffers_begin(iobuf_.data()));
    
    // Handle redirection - i.e. 301, 302, 
    if(response_.status_code >= 300 && response_.status_code < 400){
      
      entity::url url;
      auto iter = find_header(response_.headers, "Location"); 

      if(iter == response_.headers.end())
        goto BAD_MESSAGE;
      
      if(!parser::parse_url(iter->value.begin(), iter->value.end(), url))
        goto BAD_MESSAGE;
     
      // run(url.host, determin_service(url), 

      /*
      boost::system::error_code ec;
      socket_.shutdown(tcp::socket::shutdown_both, ec);
      socket_.close();
      */
      
    }
    agent2_observable_interface::ready_for_read::notify(
      response_, socket_, iobuf_);
    
    /*
    // Start reading remaining data until EOF.
    asio::async_read(socket_, response_,
        asio::transfer_at_least(1),
        boost::bind(&agent2::handle_read_content, this,
          asio::placeholders::error));
          */
BAD_MESSAGE:
    agent2_observable_interface::error::notify(
      sys::error_code(
        sys::errc::bad_message,
        sys::system_category()));
  }else{
    agent2_observable_interface::error::notify(err);
  }
}


} // namespace http


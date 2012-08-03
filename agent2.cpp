#include <iostream>
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include "agent2.hpp"

namespace http {
 
agent2::agent2(asio::io_service& io_service)
  : resolver_(io_service),
    socket_(io_service)
{}

void agent2::run(std::string const &server, 
         std::string const &service, 
         entity::request const &request,
         std::string const &body)
{
  // store for redirection
  request_ = request;

  std::ostream request(&iobuf_);
  request.flush();
  request << header;

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
    std::istream response_stream(&iobuf_);
    std::string http_version;

    response_stream >> http_version;
    response_stream >> response_.status_code;
    std::getline(response_stream, response_.message);

    if (!response_stream || http_version.substr(0, 5) != "HTTP/"){
      agent2_observable_interface::error::notify(
        sys::error_code(
          sys::errc::bad_message,
          sys::system_category()));
      return;
    }
    
    request_.http_version_major =
      lexical_cast<int>(http_version.substr(5,1));

    request_.http_version_minor =
      lexical_cast<int>(http_version.substr(7,1));

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
  if (!err) {
    // Process the response headers.
    std::istream response_stream(&iobuf_);
    std::string header;
    while (std::getline(response_stream, header) && header != "\r")
      std::cout << header << "\n";
    std::cout << "\n";

    // Write whatever content we already have to output.
    if (response_.size() > 0)
      std::cout << &response_;
    
    /*
    // Start reading remaining data until EOF.
    asio::async_read(socket_, response_,
        asio::transfer_at_least(1),
        boost::bind(&agent2::handle_read_content, this,
          asio::placeholders::error));
          */
  }else{
    agent2_observable_interface::error::notify(err);
  }
}

void handle_read_content(const boost::system::error_code& err)
{
  if (!err)
  {
    // Write all of the data that has been read so far.
    std::cout << &response_;

    // Continue reading remaining data until EOF.
    asio::async_read(socket_, response_,
        asio::transfer_at_least(1),
        boost::bind(&agent2::handle_read_content, this,
          asio::placeholders::error));
  }
  else if (err != asio::error::eof)
  {
    std::cout << "Error: " << err << "\n";
  }
}

} // namespace http


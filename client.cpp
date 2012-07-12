#include "client.hpp"

#include <iostream>
#include <istream>
#include <ostream>

namespace http {

client::client(boost::asio::io_service& io_service,
               const std::string& server, const std::string& path)
//: resolver_(io_service), socket_(io_service),
{
  resolver_.reset(new tcp::resolver(io_service));
  socket_.reset(new tcp::socket(io_service));
  request_.reset(new boost::asio::streambuf);
  response_.reset(new boost::asio::streambuf);

  std::ostream request_stream(request_.get());
  request_stream << "GET " << path << " HTTP/1.0\r\n";
  request_stream << "Host: " << server << "\r\n";
  request_stream << "Accept: */*\r\n";
  request_stream << "Connection: close\r\n\r\n";

  tcp::resolver::query query(server, "http");
  resolver_->async_resolve(
    query, 
    boost::bind(&client::handle_resolve, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::iterator
               )
    );
}

void client::handle_resolve(const boost::system::error_code& err,
                tcp::resolver::iterator endpoint_iterator)
{
  if(!err){
    boost::asio::async_connect(
      *socket_, 
      endpoint_iterator, 
      boost::bind(
        *this,
        boost::asio::placeholders::error
        )
      );
  }else{
    std::cout << "Error: " << err.message() << "\n";
  }
}

#include "yield.hpp"

void client::operator()(
  boost::system::error_code err, 
  std::size_t length)
{
   if(!err){
     reenter (this) {
      yield boost::asio::async_write(*socket_, *request_, *this);
      yield boost::asio::async_read_until(*socket_, *response_, "\r\n", *this);
      {
        // Check that response is OK.
        std::cout<<response_->size()<<"\n";
        std::istream response_stream(response_.get());
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);
        if (!response_stream || http_version.substr(0, 5) != "HTTP/")
        {
          std::cout << "Invalid response\n";
          return;
        }
        if (status_code != 200)
        {
          std::cout << "Response returned with status code ";
          std::cout << status_code << "\n";
          return;
        }
      }

      // Read the response headers, which are terminated by a blank line.
      yield boost::asio::async_read_until(*socket_, *response_, 
                                          "\r\n\r\n", *this);
      {
        // Process the response headers.
        std::istream response_stream(response_.get());
        std::string header;
        while (std::getline(response_stream, header) && header != "\r")
          std::cout << header << "\n";
        std::cout << "\n";
      }

      // Write whatever content we already have to output.
      if (response_->size() > 0)
        std::cout << response_.get();

      while(1){
        // Start reading remaining data until EOF.
        yield boost::asio::async_read(*socket_, *response_, 
                                      boost::asio::transfer_at_least(1), *this);
        // Write all of the data that has been read so far.
        std::cout << response_.get();
      }
      
     } // reenter end
   }else if(err != boost::asio::error::eof){
     std::cout << "Error: " << err.message() << "\n";
   }
}

#include "unyield.hpp"

} //namespace http::client

int main(int argc, char **argv)
{
  try
  {
    if (argc != 3)
    {
      std::cout << "Usage: async_client <server> <path>\n";
      std::cout << "Example:\n";
      std::cout << "  async_client www.boost.org /LICENSE_1_0.txt\n";
      return 1;
    }

    boost::asio::io_service io_service;
    http::client c(io_service, argv[1], argv[2]);
    io_service.run();
  }
  catch (std::exception& e)
  {
    std::cout << "Exception: " << e.what() << "\n";
  }
}

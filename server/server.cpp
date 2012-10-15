#include "server.hpp"
#include <boost/bind.hpp>
#include <signal.h>

namespace http {

namespace asio = boost::asio;

server::server(
  boost::asio::io_service &io_service,
  connection_manager &cm,
  std::string const &address,
  std::string const &port)
  : io_service_(io_service),
    connection_manager_(cm)
{
  signals_.add(SIGINT);
  signals_.add(SIGTERM);
#if defined(SIGQUIT)
  signals_.add(SIGQUIT);
#endif 
  signals_.async_wait(boost::bind(&server::handle_stop, this)); 
  
  boost::asio::ip::tcp::resolver resolver(io_service_);
  boost::asio::ip::tcp::resolver::query query(address, port);
  boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
  acceptor_.listen();

  start_accept();
}

//void server::run()
//{}

void server::start_accept()
{
  new_connection_.reset(new connection(io_service_,
        connection_manager_, request_handler_));
  acceptor_.async_accept(new_connection_->socket(),
      boost::bind(&server::handle_accept, this,
        boost::asio::placeholders::error));
}

void server::handle_accept(const boost::system::error_code& e)
{
  if (!acceptor_.is_open())
    return;

  if(!e){
    connection_ptr_.reset(
      new connection(
        io_service, connection::OWNER::SERVER)
      );

    connection_manager_.add(connection_ptr_);
    asio::async_read_until(
      connection_ptr_->socket(), 
      connection_ptr_->io_buffer(), 
      "\r\n",
      boost::bind(
        &server::handle_read_status_line, this,
        asio::placeholders::error)
      ); 
  }else{
    interface::on_response::notify(err, request_, connection_ptr_);
  }

  // start_accept();
}

void server::handle_read_status_line(
  boost::system::error_code const &err)
{
  if (!err) {
    boost::system::error_code http_err;
    // Check that response is OK.
    auto beg(asio::buffers_begin(connection_ptr_->io_buffer().data())), 
         end(asio::buffers_end(connection_ptr_->io_buffer().data()));
    
    if(!parser::parse_request_first_line(beg, end, request_)){
      http_err.assign(sys::errc::bad_message, sys::system_category());
      interface::on_response::notify(http_err, request_, connection_ptr_);
      return;
    }
    
    connection_ptr_->io_buffer().consume(
      beg - asio::buffers_begin(connection_ptr_->io_buffer().data()));

    // Read the response headers, which are terminated by a blank line.
    deadline_.expires_from_now(boost::posix_time::seconds(10));
    asio::async_read_until(
      connection_ptr_->socket(), 
      connection_ptr_->io_buffer(), 
      "\r\n\r\n",
      boost::bind(
        &server::handle_read_headers, this,
        asio::placeholders::error));
  } else {
    interface::on_response::notify(err, request_, connection_ptr_);
  }
}

void server::handle_read_headers(boost::system::error_code &err)
{
  if (!err) {
    // Process the request headers.
    auto beg(asio::buffers_begin(connection_ptr_->io_buffer().data())), 
         end(asio::buffers_end(connection_ptr_->io_buffer().data()));

    if(!parser::parse_header_list(beg, end, request_.headers)){
      interface::on_response::notify(
        boost::system::error_code(
          sys::errc::bad_message,
          sys::system_category()), 
        request_, 
        connection_ptr_);
    }else{
      connection_ptr_->io_buffer().consume(
        beg - asio::buffers_begin(connection_ptr_->io_buffer().data()));
    }
  }else{
    interface::on_response::notify(err, request_, connection_ptr_);
  }
  start_accept();
  return;
}

void server::handle_stop()
{
  acceptor_.close();
  connection_manager_.stop_all();
}

} // namespace http

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include "parser.hpp"
#include "session.hpp"
#include "connection_manager.hpp"
#include "uri_dispatcher.hpp"
#include <iostream>

namespace asio = boost::asio;
namespace sys = boost::system;

namespace http {

session::session(
  boost::asio::io_service &io_service, 
  connection_manager &cm, 
  connection_ptr c,
  uri_dispatcher& dispatcher)
: connection_manager_(cm),
  connection_ptr_(c),
  deadline_(io_service),
  stop_check_deadline_(false),
  dispatcher_(dispatcher)
{}

session::~session()
{
  std::cerr << "session is freed\n";
  //connection_ptr_->close();
  //connection_manager_.remove(connection_ptr_);
}

void session::start()
{
  if(connection_ptr_ && connection_ptr_->is_open()){
    connection_manager_.add(connection_ptr_);  
    
    deadline_.async_wait(
      boost::bind(
        &session::check_deadline, 
        shared_from_this()));

    deadline_.expires_from_now(boost::posix_time::seconds(10));


    asio::async_read_until(
      connection_ptr_->socket(), 
      connection_ptr_->io_buffer(), 
      "\r\n",
      boost::bind(
        &session::handle_read_status_line, 
        shared_from_this(),
        asio::placeholders::error)
      );
  }
}

void session::handle_read_status_line(
  boost::system::error_code const &err)
{
  if(!connection_ptr_->is_open() || stop_check_deadline_) return;

  if (!err) {
    boost::system::error_code http_err;
    // Check that response is OK.
    auto beg(asio::buffers_begin(connection_ptr_->io_buffer().data())), 
         end(asio::buffers_end(connection_ptr_->io_buffer().data()));
    
    if(!parser::parse_request_first_line(beg, end, request_)){
      http_err.assign(sys::errc::bad_message, sys::system_category());
      notify(http_err);
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
        &session::handle_read_headers, 
        shared_from_this(),
        asio::placeholders::error));
  } else {
    notify(err);
  }
}

void session::handle_read_headers(boost::system::error_code const &err)
{
  sys::error_code err_rt(err);

  if(!connection_ptr_->is_open() || stop_check_deadline_) return;

  if (!err_rt) {
    // Process the request headers.
    auto beg(asio::buffers_begin(connection_ptr_->io_buffer().data())), 
         end(asio::buffers_end(connection_ptr_->io_buffer().data()));

    if(!parser::parse_header_list(beg, end, request_.headers)){
      err_rt.assign(sys::errc::bad_message, sys::system_category());
    }else{
      connection_ptr_->io_buffer().consume(
        beg - asio::buffers_begin(connection_ptr_->io_buffer().data()));
    }
  }
  
  deadline_.cancel();
  stop_check_deadline_ = true;
  notify(err_rt);
  return;
}

void session::check_deadline()
{
  using namespace boost::system;

  if(!connection_ptr_->is_open() || stop_check_deadline_) return;
 
  /*
  if(deadline_.expires_at() <= asio::deadline_timer::traits_type::now()) {
    
    notify(error_code(errc::timed_out, system_category()));

    connection_manager_.remove(connection_ptr_);
    connection_ptr_.reset();
    deadline_.expires_at(
      boost::posix_time::pos_infin);
  }
  */
}

void session::notify(boost::system::error_code const &err)
{
  dispatcher_(request_.query.path).notify(
    err, request_, connection_ptr_);
}

} // namespace http

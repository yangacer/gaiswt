#include "parser.hpp"
#include <istream>
#include <ostream>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/lexical_cast.hpp>
#include "agent.hpp"
#include "utility.hpp"

#define GAISWT_MAXIMUM_REDIRECT_COUNT 5

namespace http {

namespace asio = boost::asio;

agent::agent(asio::io_service& io_service)
  : resolver_(io_service),
    socket_(io_service),
    redirect_count_(0),
    deadline_(io_service),
    stop_(false)
{}

agent::~agent()
{}

boost::asio::streambuf &
agent::front_data()
{
  return iobuf_;
}

agent::tcp::socket &
agent::socket()
{
  return this->socket_;
}

void agent::run(std::string const &server, 
         std::string const &service, 
         entity::request const &request,
         std::string const &body)
{
  
  // store for redirection
  request_ = request;
  
  iobuf_.consume(iobuf_.in_avail());
  std::ostream request_stream(&iobuf_);
  request_stream << request;
  
  // std::cout << "request dump beg -----\n" ;
  // std::cout << request_;
  // std::cout << "request dump end -----\n" ;

  tcp::resolver::query query(server, service);
  resolver_.async_resolve(query,
      boost::bind(&agent::handle_resolve, this,
        asio::placeholders::error,
        asio::placeholders::iterator));

  deadline_.async_wait(boost::bind(&agent::check_deadline, this));
}


void agent::handle_resolve(
  const boost::system::error_code& err,
  tcp::resolver::iterator endpoint_iterator)
{
  if(stop_) return;
  
  if (!err && endpoint_iterator != tcp::resolver::iterator()) {
    deadline_.expires_from_now(boost::posix_time::seconds(5));
    asio::async_connect(
      socket_, endpoint_iterator,
      boost::bind(
        &agent::handle_connect, 
        this, asio::placeholders::error, endpoint_iterator));
  }else{
    agent_interface::error::notify(err, *this);
  }
}

void agent::handle_connect(
  const boost::system::error_code& err,
  tcp::resolver::iterator endpoint_iterator)
{
  if(stop_) return;
  
  if (!socket_.is_open()){
    handle_resolve(boost::system::error_code(), ++endpoint_iterator);
  }else if (!err){
    asio::async_write(socket_, iobuf_,
        boost::bind(
          &agent::handle_write_request, this,
          asio::placeholders::error,
          asio::placeholders::bytes_transferred
          ));
  }else{
    agent_interface::error::notify(err, *this);
  }
}

void agent::handle_write_request(
  boost::system::error_code const & err,
  boost::uint32_t len)
{
  if(stop_) return;
  
  if (!err){
    iobuf_.consume(len);
    deadline_.expires_from_now(boost::posix_time::seconds(5));
    asio::async_read_until(
      socket_, iobuf_, "\r\n",
      boost::bind(&agent::handle_read_status_line, this,
                  asio::placeholders::error));
  }else{
    agent_interface::error::notify(err, *this);
  }
}

void agent::handle_read_status_line(const boost::system::error_code& err)
{
  namespace sys = boost::system;

  using boost::lexical_cast;

  if(stop_) return;
  
  if (!err) {
    // Check that response is OK.
    auto beg(asio::buffers_begin(iobuf_.data())), 
         end(asio::buffers_end(iobuf_.data()));
    
    if(!parser::parse_response_first_line(beg, end, response_)){
      agent_interface::error::notify(
        sys::error_code(
          sys::errc::bad_message,
          sys::system_category()), 
        *this);
      return;
    }
    
    iobuf_.consume(beg - asio::buffers_begin(iobuf_.data()));

    // Read the response headers, which are terminated by a blank line.
    deadline_.expires_from_now(boost::posix_time::seconds(10));
    asio::async_read_until(socket_, iobuf_, "\r\n\r\n",
        boost::bind(&agent::handle_read_headers, this,
          asio::placeholders::error));
  } else {
    agent_interface::error::notify(err, *this);
  }
}

void agent::handle_read_headers(const boost::system::error_code& err)
{
  if (!err) {
    // Process the response headers.
    auto beg(asio::buffers_begin(iobuf_.data())), 
         end(asio::buffers_end(iobuf_.data()));

    if(!parser::parse_header_list(beg, end, response_.headers))
      goto BAD_MESSAGE;

    //std::cout << "Header consumed: " << beg - asio::buffers_begin(iobuf_.data()) << "\n";
    iobuf_.consume(beg - asio::buffers_begin(iobuf_.data()));
    
    // TODO better log
    // std::cout << "response dump beg ----\n";
    // std::cout << response_;
    // std::cout << "response dump end ----\n";

    // Handle redirection - i.e. 301, 302, 
    if(response_.status_code >= 300 && response_.status_code < 400){
      redirect();
    }else{
      stop_ = true;
      deadline_.cancel();
      agent_interface::ready_for_read::notify(
        response_, *this);
    }
  }else{
    agent_interface::error::notify(err, *this);
  }

  return;

  namespace sys = boost::system;

BAD_MESSAGE:
  agent_interface::error::notify(
    sys::error_code(
      sys::errc::bad_message,
      sys::system_category()),
    *this);
  return;
}

void agent::redirect()
{
  namespace sys = boost::system;
  
  sys::error_code ec;

  entity::url url;
  auto iter = find_header(response_.headers, "Location"); 
  auto beg(iter->value.begin()), end(iter->value.end());

  if(GAISWT_MAXIMUM_REDIRECT_COUNT <= redirect_count_)
    goto OPERATION_CANCEL;

  if(iter == response_.headers.end())
    goto BAD_MESSAGE;

  if(!parser::parse_url(beg, end, url))
    goto BAD_MESSAGE;

  iter = find_header(request_.headers, "Host");
  iter->value = url.host;

  request_.query = url.query;

  response_.message.clear();
  response_.headers.clear();

  redirect_count_++;
  socket_.close();

  run(url.host, determine_service(url), request_, "");

  return;

BAD_MESSAGE:
  ec.assign(sys::errc::bad_message,
            sys::system_category());
  agent_interface::error::notify(ec, *this);
  return;

OPERATION_CANCEL:
  ec.assign(sys::errc::operation_canceled,
            sys::system_category()); 
  return;
  agent_interface::error::notify(ec, *this);
}

void agent::check_deadline()
{
  using namespace boost::system;

  if(stop_) return;
  
  if(deadline_.expires_at() <= asio::deadline_timer::traits_type::now()) {
    agent_interface::error::notify(
      error_code(errc::timed_out, system_category()), 
      *this);
    socket_.close();
    deadline_.expires_at(boost::posix_time::pos_infin);
  }

  deadline_.async_wait(boost::bind(&agent::check_deadline,this));
}

} // namespace http


#include "server.hpp"
#include <boost/asio/placeholders.hpp>
#include <boost/bind.hpp>
#include <signal.h>
#include "connection_manager.hpp"
#include "session.hpp"

namespace http {

namespace asio = boost::asio;

// TODO isolated session class is in need
//
server::server(
  boost::asio::io_service &io_service,
  connection_manager &cm,
  std::string const &address,
  std::string const &port)
  : io_service_(io_service),
    signals_(io_service),
    acceptor_(io_service),
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
  connection_ptr_.reset(
    new connection(
      io_service_,connection::OWNER::SERVER)
    );
  
  acceptor_.async_accept(connection_ptr_->socket(),
      boost::bind(&server::handle_accept, this,
        boost::asio::placeholders::error));
}

void server::handle_accept(const boost::system::error_code& e)
{
  if (!acceptor_.is_open())
    return;

  if(!e){
    boost::shared_ptr<session> session_ptr(
      new session(
        io_service_, 
        connection_manager_,
        connection_ptr_));
  }

  start_accept();
}

void server::handle_stop()
{
  acceptor_.close();
  connection_manager_.stop(connection::OWNER::SERVER);
}


} // namespace http

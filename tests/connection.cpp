#include "connection.hpp"
#include "connection_manager.hpp"

#include <boost/asio.hpp>
#include <cassert>

int main()
{
  using namespace http;

  connection_manager manager;
  boost::asio::io_service io_service;

  connection_ptr 
    conn_1(new connection(io_service, manager, connection::OWNER::SERVER)), 
    conn_2(new connection(io_service, manager, connection::OWNER::AGENT));
  
  manager.add(conn_1);
  manager.add(conn_2);
  assert(manager.count(connection::OWNER::SERVER) == 1 );
  assert(manager.count(connection::OWNER::AGENT) == 1 );

  manager.remove(conn_2);
  assert(manager.count(connection::OWNER::SERVER) == 1 );
  assert(manager.count(connection::OWNER::AGENT) == 0 );
  
  manager.change_owner(conn_1, connection::OWNER::AGENT);
  assert(manager.count(connection::OWNER::SERVER) == 0 );
  assert(manager.count(connection::OWNER::AGENT) == 1 );

  manager.stop(connection::OWNER::SERVER);
  manager.stop(connection::OWNER::AGENT);
  assert(manager.count(connection::OWNER::SERVER) == 0 );
  assert(manager.count(connection::OWNER::AGENT) == 0 );

  return 0;  
}

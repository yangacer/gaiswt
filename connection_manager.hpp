#ifndef GAISWT_CONNECTION_MANAGER_HPP_
#define GAISWT_CONNECTION_MANAGER_HPP_

#include <boost/noncopyable.hpp>
#include <set>
#include "connection.hpp"

namespace http {
  
class connection_manager 
{
public: 
  void add(connection_ptr c);
  void remove(connection_ptr c);

  void change_owner(
    connection_ptr c, 
    connection::OWNER owner);

  void stop(connection::OWNER owner);
  int count(connection::OWNER owner);
  
private:
  std::set<connection_ptr> 
    server_stream_,
    agent_stream_;
};

} // namespace http

#endif

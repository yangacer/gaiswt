#include "connection_manager.hpp"

namespace http {

void 
connection_manager::add(connection_ptr c)
{
  switch(c->owner()){
  case connection::OWNER::SERVER:
    server_stream_.insert(c);
  break;
  case connection::OWNER::AGENT:
    agent_stream_.insert(c);
  break;
  }
}

void
connection_manager::remove(connection_ptr c)
{
  switch(c->owner()){
  case connection::OWNER::SERVER:
    server_stream_.erase(c);
  break;
  case connection::OWNER::AGENT:
    agent_stream_.erase(c);
  break;
  }
}

void
connection_manager::change_owner(
  connection_ptr c, 
  connection::OWNER o)
{
  if(c->owner() != o){
    switch(c->owner()){
    case connection::OWNER::SERVER:
      server_stream_.erase(c);
      agent_stream_.insert(c);
    break;
    case connection::OWNER::AGENT:
      agent_stream_.erase(c);
      server_stream_.insert(c);
    break;
    }
    c->owner(o);
  }
}

void connection_manager::stop(connection::OWNER o)
{
  switch(o){
  case connection::OWNER::SERVER:
    for(auto i=server_stream_.begin();i!=server_stream_.end();++i)
      (*i)->close();
    server_stream_.clear();
  break;
  case connection::OWNER::AGENT:
    for(auto i=agent_stream_.begin();i!=agent_stream_.end();++i)
      (*i)->close();
    agent_stream_.clear();
  break;
  }
}

int connection_manager::count(connection::OWNER o) const
{
  switch(o){
  case connection::OWNER::SERVER:
    return server_stream_.size();
  case connection::OWNER::AGENT:
    return agent_stream_.size();
  }
  return 0;
}

} // namespace http

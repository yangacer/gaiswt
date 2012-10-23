#include "map_ele.hpp"
#include "region.hpp"

namespace detail{

map_ele_t::map_ele_t()
: max_size_(0), is_open_(false)
{}

map_ele_t::map_ele_t(std::string const& fname)
: mfile(
    fname.c_str(), 
    boost::interprocess::read_write), 
  max_size_(0),
  is_open_(true)
{
  
}

void map_ele_t::open(std::string const& fname)
{
  if(is_open_) return;

  mfile = boost::interprocess::file_mapping(
    fname.c_str(), 
    boost::interprocess::read_write);

  for(auto i=this->begin();i!=this->end();++i){
    if(!i->get()->get_file_mapping())
      i->get()->use_file_mapping(mfile);
  }

  is_open_ = true;
}

} // namespace detail

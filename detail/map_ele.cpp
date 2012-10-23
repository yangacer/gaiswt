#include "map_ele.hpp"

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
  mfile = boost::interprocess::file_mapping(
    fname.c_str(), 
    boost::interprocess::read_write);

  is_open_ = true;
}

} // namespace detail

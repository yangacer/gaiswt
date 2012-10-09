#include "map_ele.hpp"

namespace detail{

map_ele_t::map_ele_t(std::string const& fname)
: mfile(
    fname.c_str(), 
    boost::interprocess::read_write), 
  max_size_(0)
{}

} // namespace detail

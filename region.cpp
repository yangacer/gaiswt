#include "region.hpp"
#include <iostream>

namespace ipc = boost::interprocess;

region_impl_t::region_impl_t()
: base_t(), 
  mms_(0),
  file_(0), 
  committed_(0), 
  offset_(0), size_(0)
{}

region_impl_t::region_impl_t(
  mmstore &mms,
  ipc::file_mapping const& mem, 
  mmstore::mode_t mode, 
  boost::int64_t off, 
  boost::uint32_t size,
  void* const addr)
: base_t(mem, ipc::read_write, off, size, addr),
  mms_(&mms),
  file_(&mem),
  mode_(mode), committed_(false),
  offset_(off), size_(size)
{}

region_impl_t::~region_impl_t()
{
  //std::cout << "region destroyed\n";
}

void region_impl_t::commit(boost::uint32_t n)
{ 
  if(mode_ == mmstore::read)
    return;

  committed_ += n; 
  assert( committed_ <= get_size() && "region overflow");
}

void region_impl_t::rollback(boost::uint32_t n)
{
  if(mode_ == mmstore::read)
    return;

  committed_ -= n;
  assert( committed_ >= 0 && "region underflow");
}

region_impl_t::raw_region_t region_impl_t::buffer()
{
  using std::make_pair;

  if(!get_address()) 
    goto FAIL;

  switch(mode_){
  case mmstore::read:
    return make_pair(get_address(), committed_);
  break;
  case mmstore::write:
    return make_pair(
      (char*)get_address() + committed_, get_size() - committed_);
  break;
  }
FAIL:
  return make_pair((void*)0, 0);
}

void region_impl_t::mode(mmstore::mode_t m)
{ mode_ = m; }

void region_impl_t::map()
{
  if(!is_mapped()){
    base_t tmp(*file_, ipc::read_write, offset_, size_);
    swap(tmp);
  }
}

void region_impl_t::unmap()
{
  if(is_mapped()){
    base_t tmp;
    swap(tmp);
  }
}

mmstore& region_impl_t::mms()
{ return *mms_; }

mmstore::mode_t region_impl_t::mode() const
{ return mode_; }

boost::uint32_t region_impl_t::committed() const
{ return committed_; }

bool region_impl_t::is_mapped() const
{ return get_address() != 0; }

boost::uint32_t region_impl_t::get_size() const
{ return size_; }

boost::int64_t region_impl_t::get_offset() const
{ return offset_; }

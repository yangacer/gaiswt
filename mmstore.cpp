#include "mmstore.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <cassert>
#include <cstdio>
#include <cerrno>
#include <algorithm>
#include <boost/bind.hpp>
#include <stdexcept>

namespace ipc = boost::interprocess;
namespace sys = boost::system;

namespace detail{

  typedef boost::shared_ptr<ipc::mapped_region> region_ptr;

  bool is_overlap(
    region_ptr rp, 
    boost::uint64_t offset,
    boost::uint32_t size)
  {
    boost::uint64_t 
      end = offset + size,
      rp_off = rp->get_offset(),
      rp_end = rp_off + rp->get_size();
    
    return 
      (offset < rp_end && offset >= rp_off) ||
      (end < rp_end && end >= rp_off)
      ;
  }

  inline boost::uint32_t get_padded_size(boost::uint32_t size)
  { 
    boost::uint32_t rt =
      ((size / ipc::mapped_region::get_page_size()) * 
        ipc::mapped_region::get_page_size());
    if(rt >= size)
      return rt;
    else
      return rt + ipc::mapped_region::get_page_size();
   }
} // namespace detail

// ---------- region_impl_t -------------

struct region_impl_t 
: boost::interprocess::mapped_region
{
  typedef ipc::mapped_region base_t;

  region_impl_t(){}

  template<typename M, typename A>
    region_impl_t(
      M const& mem, A mode, 
      boost::int64_t off =0, boost::uint32_t size=0,
      void* const addr = 0 )
    : base_t(),
      committed_(false)
  { swap(base_t(mem, mode, off, size, addr)); }

  void commit(boost::uint32_t n)
  { 
    committed_ += n; 
    assert( committed_ <= get_size() &&
            "region overflow");
  }

  boost::uint32_t committed() const
  { return committed_; }

private:
  boost::uint32_t committed_;
};

// ---------- mmstore::map_ele_t impl --------------

struct map_ele_t
{
  typedef boost::interprocess::file_mapping file_mapping;

  map_ele_t(std::string const& fname)
    : mfile()
  {
    new (&mfile) file_mapping(fname.c_str(), ipc::read_write);
  }

  file_mapping mfile;

  std::set<
    boost::shared_ptr<region_impl_t> 
    > regions;
};

// --------- mmstore::region impl ----------

mmstore::region::region()
  : impl_()
{}

mmstore::region::region(
  boost::shared_ptr<region_impl_t> impl)
  : impl_(impl)
{}

mmstore::region::raw_region_t
mmstore::region::buffer()
{
  if(!impl_) 
    return std::make_pair(
      static_cast<void*>(0), 0);

  return std::make_pair(
    impl_->get_address(),
    impl_->get_size());
}

void 
mmstore::region::commit(boost::uint32_t n)
{ impl_->commit(n); }

boost::uint32_t 
mmstore::region::size() const
{ return impl_->get_size(); }

boost::uint64_t
mmstore::region::offset() const
{ return impl_->get_offset(); }

long
mmstore::region::shared_count() const
{ return impl_.use_count(); }

mmstore::region::operator void*() const
{ return impl_.get(); }

// ---------- task_t impl ------------

struct task_t 
{
  task_t(
    mmstore::region &r, 
    std::string const& file, 
    mode_t mode,
    boost::uint64_t offset, 
    mmstore::completion_handler_t handler)
    : region_(r), file_(file), mode_(mode),
    offset_(offset), handler_(handler)
  {}

  mmstore::region &region_;
  std::string file_;
  mode_t mode_;
  boost::uint64_t offset_;
  mmstore::completion_handler_t handler_;
};

// ---------- mmstore impl --------------

mmstore::mmstore(
  std::string dir, 
  std::string maximum_memory, 
  std::string concurrency_level)
: prefix_(dir), current_used_memory_(0)
{
  using boost::lexical_cast;
  
  maximum_memory_ = 
    boost::lexical_cast<boost::uint64_t>(maximum_memory);

  concurrency_level_ =
    boost::lexical_cast<boost::uint32_t>(concurrency_level);

 if(!maximum_region_size())
    throw std::invalid_argument("Maximum region size is zero");
  
}

//mmstore::~mmstore()
//{}

void
mmstore::create(std::string const &name)
{
  using boost::shared_ptr;
  using boost::uint32_t;
  using boost::uint64_t;

  shared_ptr<map_ele_t> &sp(storage_[name]);
  
  if(!sp){ // new file
    // TODO filename checking 
    FILE* fp = fopen(name.c_str(), "w+b");
    if(!fp) 
      throw sys::system_error(
        sys::error_code(errno, sys::system_category())
        );
    fclose(fp);
    sp.reset(new map_ele_t(name));
  }
}

mmstore::region
mmstore::write_region(std::string const &name, boost::uint64_t offset)
{
  using boost::shared_ptr;
  using boost::uint32_t;
  using boost::uint64_t;

  shared_ptr<region_impl_t> rgn_ptr;
  shared_ptr<map_ele_t> &sp(storage_[name]);

  if(!sp) return region(rgn_ptr);

  uint32_t size;
  uint32_t avail;

  try{
    avail = boost::numeric_cast<uint32_t, uint64_t>(available_memory());
    size = std::min(maximum_region_size(), avail);
  }catch(boost::numeric::bad_numeric_cast &e){
    size = maximum_region_size();
  }

  if(sp->regions.size()){ // search overlap
    typedef std::set<shared_ptr<region_impl_t> >::iterator iter_t;
    iter_t rt;
    rt = std::find_if(
      sp->regions.begin(), sp->regions.end(),
      boost::bind(&detail::is_overlap, _1, offset, size ));

    if(rt != sp->regions.end())
      return region(rgn_ptr);
  }

  rgn_ptr.reset(
      new region_impl_t(
        sp->mfile, ipc::read_write, offset, size
        ));

  sp->regions.insert(rgn_ptr);
  
  current_used_memory_ += detail::get_padded_size(size);

  return region(rgn_ptr);
}

void mmstore::async_get_region(
  region &r, 
  std::string const& file, 
  mode_t mode,
  boost::uint64_t offset,
  completion_handler_t handler)
{}

boost::uint64_t
mmstore::maximum_memory() const
{ return maximum_memory_; }

boost::uint32_t 
mmstore::maximum_region_size() const
{ 
  return detail::get_padded_size(maximum_memory_ / concurrency_level_);
}

boost::uint64_t
mmstore::current_used_memory() const
{  return current_used_memory_; }

boost::uint64_t
mmstore::available_memory() const
{
  return maximum_memory_  - current_used_memory_;
}


#include "mmstore.hpp"
#include "region.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <cassert>
#include <cstdio>
#include <cerrno>
#include <algorithm>
#include <boost/bind.hpp>
#include <stdexcept>
#include <sys/types.h>
#include <sys/stat.h>

namespace ipc = boost::interprocess;
namespace sys = boost::system;
using boost::shared_ptr;
using boost::uint32_t;
using boost::int64_t;

namespace detail{

  typedef boost::shared_ptr<region_impl_t> region_ptr;

  bool is_overlap(
    region_ptr rp, 
    boost::int64_t offset,
    boost::uint32_t size)
  {
    boost::int64_t 
      end = offset + size,
      rp_off = rp->get_offset(),
      rp_end = rp_off + rp->get_size();
    
    return 
      (offset < rp_end && offset >= rp_off) ||
      (end < rp_end && end >= rp_off)
      ;
  }
  
  bool is_between(region_ptr rp, boost::int64_t offset)
  {
    boost::int64_t 
      rp_off = rp->get_offset(),
      rp_end = rp_off + rp->get_size();

    return offset < rp_end && offset >= rp_off;
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

#if defined(__WIN32__) || defined(__WIN64__)
#include <io.h>
  int truncate(char const *name, boost::int64_t size)
  {
    int fd = -1;
    if(_sopen_s(&fd, name, _O_RDWR))
      throw sys::system_error(
        sys::error_code(errno, sys::system_category()));
    if(_chsize_s(fd, size))
      throw sys::system_error(
        sys::error_code(errno, sys::system_category()));
     _close(fd);
     return 0;
  }
#endif

#ifdef __GNUC__
#include <unistd.h>
#endif
  void truncate_if_too_small(std::string const &name, boost::int64_t size)
  {
    int result = 0;
#ifdef __GNUC__
    struct stat sb;
    result = stat(name.c_str(), &sb);
#elif defined (__WIN32__) || defined(__WIN64__)
    struct _stat sb;
    result = stat(name.c_str(), &sb);
#else
    struct STRUCT_STAT_NOT_FOUND sb;
#endif
    if(result) 
      throw sys::system_error(
        sys::error_code(errno, sys::system_category()));

    if(sb.st_size < size)
      if(truncate(name.c_str(), size))
        throw sys::system_error(
          sys::error_code(errno, sys::system_category()));
  }


} // namespace detail

// ---------- mmstore::map_ele_t impl --------------

struct map_ele_t
{
  typedef boost::interprocess::file_mapping file_mapping;

  map_ele_t(std::string const& fname)
    : mfile(fname.c_str(), ipc::read_write), max_size_(0)
  {}

  file_mapping mfile;
  boost::int64_t max_size_;

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

mmstore::region::~region()
{}

mmstore::region::raw_region_t
mmstore::region::buffer()
{ return impl_->buffer(); }

void 
mmstore::region::commit(boost::uint32_t n)
{ impl_->commit(n); }

void 
mmstore::region::rollback(boost::uint32_t n)
{ impl_->rollback(n); }

boost::uint32_t
mmstore::region::committed() const
{ return impl_->committed(); }

boost::uint32_t 
mmstore::region::size() const
{ return impl_->get_size(); }

boost::int64_t
mmstore::region::offset() const
{ return impl_->get_offset(); }

long
mmstore::region::shared_count() const
{ return impl_.use_count(); }

mmstore::region::operator void* const() const
{ return impl_.get(); }

// ---------- task_t impl ------------

struct task_t 
{
  task_t(
    mmstore::region &r, 
    std::string const& name, 
    mmstore::mode_t mode,
    boost::int64_t offset, 
    mmstore::completion_handler_t handler)
    : region(r), name(name), mode(mode),
    offset(offset), handler(handler)
  {}

  mmstore::region &region;
  std::string name;
  mmstore::mode_t mode;
  boost::int64_t offset;
  mmstore::completion_handler_t handler;
};

// ---------- mmstore impl --------------

mmstore::mmstore(
  std::string maximum_memory, 
  std::string concurrency_level)
: current_used_memory_(0), page_fault_(0)
{
  using boost::lexical_cast;
  
  maximum_memory_ = 
    boost::lexical_cast<boost::int64_t>(maximum_memory);

  concurrency_level_ =
    boost::lexical_cast<boost::uint32_t>(concurrency_level);

 if(!maximum_region_size())
    throw std::invalid_argument("Maximum region size is zero");
  
}

mmstore::~mmstore()
{}

void
mmstore::create(std::string const &name)
{
  // not allow relative path
  if(std::string::npos != name.find(".."))
    throw std::invalid_argument("Relative path is not allowed");

  shared_ptr<map_ele_t> &sp(storage_[name]);
  
  if(!sp){ // new file
    FILE* fp = fopen(name.c_str(), "w+b");
    if(!fp) 
      throw sys::system_error(
        sys::error_code(errno, sys::system_category())
        );
    fclose(fp);
    sp.reset(new map_ele_t(name));
  }
}

void mmstore::stop(std::string const &name)
{
  for(auto i = pending_task_.begin(); i != pending_task_.end();++i){
    if((*i)->name == name){
      (*i)->handler(
        sys::error_code(
          sys::errc::operation_canceled,
          sys::system_category()));
      i = pending_task_.erase(i);
    }
  }
}

void mmstore::remove(std::string const &name)
{
  stop(name);
  storage_.erase(name);
}

void mmstore::async_get_region(
  region &r, 
  std::string const& name, 
  mode_t mode,
  boost::int64_t offset,
  completion_handler_t handler)
{
  shared_ptr<map_ele_t> &sp(storage_[name]);

  if(!sp)
    handler(
      sys::error_code(
        sys::errc::no_such_file_or_directory, 
        sys::system_category()
        ));

  pending_task_.push_back(
    shared_ptr<task_t>(new task_t(r, name, mode, offset, handler))
    );

  process_task();
}

void mmstore::commit_region(region &r, std::string const &file)
{
  r.impl_->mode(mmstore::read);
  r.impl_.reset();
  process_task();
}

void mmstore::set_max_size(boost::uint64_t size, std::string const &name)
{
  storage_.find(name)->second->max_size_ = size;
}

void mmstore::process_task()
{
  using boost::system::error_code;
  
  while(pending_task_.size()){
    //std::cout << "---- loop beg ----\n";
    shared_ptr<task_t> task = pending_task_.front();
    assert(storage_[task->name].get() && "no such file");

    shared_ptr<region_impl_t> rgn_ptr;
    shared_ptr<map_ele_t> &sp(storage_[task->name]);
    
    if(sp->max_size_ && task->offset >= sp->max_size_){
      pending_task_.pop_front();
      task->handler(
        error_code(
          sys::errc::result_out_of_range,  // TODO eof
          sys::system_category()));
      continue;
    }

    typedef std::set<shared_ptr<region_impl_t> >::iterator iter_t;
    iter_t rt;
    rt = std::find_if(
      sp->regions.begin(), sp->regions.end(),
      boost::bind(&detail::is_between, _1, task->offset)); 

    if(rt == sp->regions.end()){ // region not found
      if(mmstore::write == task->mode){ // write mode
      /*if(mmstore::read == task->mode){ // read mode
        pending_task_.pop_front();
        task->handler(
          error_code(sys::errc::result_out_of_range, 
                     sys::system_category()));
      }else{ // write mode
      */
        if(available_memory() < ipc::mapped_region::get_page_size())
          if(!swap_idle(maximum_region_size()))
            break;

        uint32_t size;
        uint32_t avail;
        try{
          avail = boost::numeric_cast<uint32_t, int64_t>(available_memory());
          size = std::min(maximum_region_size(), avail);
        }catch(boost::numeric::bad_numeric_cast &e){
          size = maximum_region_size();
        }
        
        assert(size > 0 && "zero size page");

        detail::truncate_if_too_small(
          sp->mfile.get_name(), task->offset + size);

        rgn_ptr.reset(
          new region_impl_t(
            sp->mfile, 
            mmstore::write, 
            task->offset, size));

        sp->regions.insert(rgn_ptr);
        task->region.impl_ = rgn_ptr;
        pending_task_.pop_front();
        current_used_memory_ += size;
        task->handler(
          error_code(sys::errc::success, sys::system_category()));
      }
    }else{ // region found
      // std::cout << "region found\n";
      rgn_ptr = *rt;
      if(!rgn_ptr->is_mapped()){
        if(available_memory() < rgn_ptr->get_size())
          if(!swap_idle(rgn_ptr->get_size()))
            break;
        rgn_ptr->map();
        current_used_memory_ += rgn_ptr->get_size();
      }
      if(mmstore::write == rgn_ptr->mode()) 
        break;
      rgn_ptr->mode(task->mode);
      task->region.impl_ = rgn_ptr;
      pending_task_.pop_front();
      task->handler(
        error_code(sys::errc::success, sys::system_category()));
    }
    // std::cout << "---- loop end ----\n";
  } // while(pending_task_.size())
  // std::cout << "exit process task loop\n";
  //dump_use_count();
}

bool mmstore::swap_idle(boost::uint32_t size)
{
  ++page_fault_;

  // find an idle page
  typedef std::map<std::string, shared_ptr<map_ele_t> >::iterator miter_t;
  typedef std::set<shared_ptr<region_impl_t> >::iterator iter_t;

  for(miter_t i=storage_.begin(); 
      i!=storage_.end() && available_memory() < size; 
      ++i)
  {
    for(iter_t j=i->second->regions.begin(); 
        j!=i->second->regions.end() && available_memory() < size ;
        ++j)
    {
      if((*j)->is_mapped() && (*j).use_count() < 2){
        // std::cout << "swap out: " << 
        //   (*i).second->mfile.get_name() << " " <<
        //   (*j)->get_offset() << "\n";

        (*j)->flush();
        (*j)->unmap();
        current_used_memory_ -= (*j)->get_size();
      }
    }
  }
  
  return (available_memory() >= size);
}

bool mmstore::is_in(std::string const &name) const
{  return storage_.count(name) != 0; }

boost::int64_t
mmstore::maximum_memory() const
{ return maximum_memory_; }

boost::uint32_t 
mmstore::maximum_region_size() const
{ 
  return detail::get_padded_size(maximum_memory_ / concurrency_level_);
}

boost::int64_t
mmstore::current_used_memory() const
{  return current_used_memory_; }

boost::int64_t
mmstore::available_memory() const
{
  return maximum_memory_  - current_used_memory_;
}

boost::int64_t 
mmstore::get_max_size(std::string const& name) const
{
  auto iter = storage_.find(name);
  return iter->second->max_size_;
}

boost::int64_t
mmstore::get_current_size(std::string const &name) const
{
  typedef std::set<shared_ptr<region_impl_t> >::const_iterator iter_t;

  std::set<shared_ptr<region_impl_t> > const &regions = 
    storage_.find(name)->second->regions;

  int64_t total(0);

  for(iter_t i=regions.begin(); i!=regions.end(); ++i){
    total += (*i)->committed();   
  }
  return total;
}

boost::uint32_t 
mmstore::page_fault() const
{ return page_fault_; }

std::ostream&
mmstore::dump_use_count(std::ostream &os) const 
{
  typedef std::map<std::string, shared_ptr<map_ele_t> >::const_iterator miter_t;
  typedef std::set<shared_ptr<region_impl_t> >::const_iterator iter_t;
  char const qt('"'), co(','), lbr('{'), rbr('}'), lbk('['), rbk(']');

  os << lbr << 
    "\"region_tuple\":" <<
    "[\"offset\",\"committed\",\"use_count\"]," <<
    "\"status\":";
  if(!storage_.size()){
    os << "null";
    goto FILE_END;
  }
  os << lbk;
  for(miter_t i=storage_.begin(); i!=storage_.end(); ++i){
    if(i != storage_.begin()) os << co; 
    os << lbr;
    os << "\"file\":" << qt << (*i->second).mfile.get_name() << qt << co;
    os << "\"regions\":" ;
    if(!i->second->regions.size()){
      os << "null";
      goto REGION_END;
    }
    os << lbk;
    for(iter_t j=i->second->regions.begin();
        j!=i->second->regions.end(); ++j)
    {
      if(j != i->second->regions.begin()) os << co;
      os << lbk << 
        (*j)->get_offset() << co <<
        (*j)->committed() << co <<
        (*j).use_count() << 
        rbk;
    }
    os << rbk;
  REGION_END:
    os << rbr;
  }
FILE_END:
  os << rbk << rbr;

  return os;
}

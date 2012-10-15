#include "mmstore.hpp"
#include "region.hpp"
#include "map_ele.hpp"
#include <boost/lexical_cast.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/system/error_code.hpp>
#include <boost/asio/error.hpp>
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

//#include <iostream>

namespace detail {

namespace ipc = boost::interprocess;
namespace sys = boost::system;
using boost::shared_ptr;
using boost::uint32_t;
using boost::int64_t;

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

boost::int64_t get_file_size(std::string const& name)
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
  return boost::numeric_cast<boost::int64_t>(sb.st_size);
}

// --------- mmstore::region impl ----------

mmstore::region::region()
  : impl_()
{}

mmstore::region::region(
  boost::shared_ptr<region_impl_t> impl)
  : impl_(impl)
{}

mmstore::region::~region()
{
  if(impl_)
    impl_->mms().commit_region(*this);
}

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
 
 //std::cerr << "mmstore constructed\n";
}

mmstore::~mmstore()
{
 //std::cerr << "mmstore destructed\n";
}

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

void mmstore::rename(std::string const &new_name, std::string const &origin)
{
  auto i = storage_.find(origin);
  if(i == storage_.end())
    return;
  storage_[new_name] = i->second;
  storage_.erase(i);
}

void mmstore::stop(std::string const &name)
{}

void mmstore::remove(std::string const &name)
{
  stop(name);
  storage_.erase(name);
}

void mmstore::import(std::string const &name)
{
  if(storage_.count(name)) return;
  create(name);
  set_max_size(name, detail::get_file_size(name));

}

boost::system::error_code 
mmstore::get_region(
    mmstore::region &region_, 
    std::string const& name, 
    mode_t mode,
    boost::int64_t offset)
{
  using namespace boost::system;

  error_code err(errc::success, system_category());
  shared_ptr<map_ele_t> &sp(storage_[name]);
  if(!sp){ // file does not exist
    err.assign(
      errc::no_such_file_or_directory,
      system_category());
  }else{ // file exists
    shared_ptr<region_impl_t> rgn_ptr;
    // eof test
    if(sp->max_size_ && offset >= sp->max_size_){
      err.assign(
        boost::asio::error::eof,
        system_category());
    }else{ // not eof
      auto target_region_iter = std::find_if(
        sp->begin(), sp->end(),
        boost::bind(&is_between, _1, offset)); 
      if(target_region_iter == sp->end()){ // region not found
        if(write == mode){
          // can we satisfy ?
          if(available_memory() < ipc::mapped_region::get_page_size() && 
             !swap_idle(maximum_region_size()))
          { // we can not
             err.assign(
               errc::resource_unavailable_try_again,
               system_category());
          }else{ // yes, we can
            uint32_t size;
            uint32_t avail;
            try{
              avail = boost::numeric_cast<uint32_t, int64_t>(available_memory());
              size = std::min(maximum_region_size(), avail);
            }catch(boost::numeric::bad_numeric_cast &e){
              size = maximum_region_size();
            }
            
            // XXX offset + size may overlap other regions
            target_region_iter = std::find_if(
              sp->begin(), sp->end(),
              boost::bind(&is_between, _1, offset+size-1));
            if(target_region_iter != sp->end())
              size = target_region_iter->get()->get_offset() - offset;

            assert(size > 0 && "zero size page");

            truncate_if_too_small(
              sp->mfile.get_name(), offset + size);

            rgn_ptr.reset(
              new region_impl_t(
                *this,
                sp->mfile, 
                mmstore::write, 
                offset, size));

            sp->insert(rgn_ptr);
            region_.impl_ = rgn_ptr;
            current_used_memory_ += size;
          }
        }else{ // read mode
          err.assign(
            errc::resource_unavailable_try_again,
            system_category());
        }
      }else{ // region found
        rgn_ptr = *target_region_iter;
        if(mmstore::write == rgn_ptr->mode()){ // acquire for writting
          err.assign(
            errc::resource_unavailable_try_again,
            system_category());
        }else{ // acquire for reading
          if(!rgn_ptr->is_mapped()){ // not mapped
            if(available_memory() < rgn_ptr->get_size() &&
               !swap_idle(rgn_ptr->get_size()))
            {
              err.assign(
                errc::resource_unavailable_try_again,
                system_category());
            }else{
              rgn_ptr->map();
              current_used_memory_ += rgn_ptr->get_size();
            }
          }
          rgn_ptr->mode(mode);
          region_.impl_ = rgn_ptr;
        } // acquire for reading
      } // region found
    } // not eof
  } // file existed
  return err;
}

void mmstore::commit_region(region &r)
{
  r.impl_->mode(mmstore::read);
  r.impl_.reset();
}

void mmstore::set_max_size(std::string const &name, boost::uint64_t size)
{
  auto m_ele = storage_.find(name)->second;
  m_ele->max_size_ = size;
  truncate_if_too_small(m_ele->mfile.get_name(), size);
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
    for(iter_t j=i->second->begin(); 
        j!=i->second->end() && available_memory() < size ;
        ++j)
    {
      if((*j)->is_mapped() && (*j).use_count() < 2){
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
  return get_padded_size(maximum_memory_ / concurrency_level_);
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
  
  auto target = storage_.find(name);
  
  if(target == storage_.end())
    return 0;

  int64_t total(0);
  map_ele_t const &regions = *(target->second);

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
    if(!i->second->size()){
      os << "null";
      goto REGION_END;
    }
    os << lbk;
    for(iter_t j=i->second->begin();
        j!=i->second->end(); ++j)
    {
      if(j != i->second->begin()) os << co;
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

} // namespace detail


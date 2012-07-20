#ifndef GAISWT_REGION_HPP_
#define GAISWT_REGION_HPP_

#include <boost/interprocess/mapped_region.hpp>
#include "mmstore.hpp"
#include <utility>
#include <boost/cstdint.hpp>

namespace ipc = boost::interprocess;

struct region_impl_t 
: boost::interprocess::mapped_region
{
  typedef boost::interprocess::mapped_region base_t;
  typedef std::pair<void*, boost::uint32_t> raw_region_t;

  region_impl_t();

  template<typename M>
  region_impl_t(
    M const& mem, mmstore::mode_t mode, 
    boost::int64_t off = 0, boost::uint32_t size = 0,
    void* const addr = 0 )
  : base_t(mem, boost::interprocess::read_write, off, size, addr),
    mode_(mode), committed_(false),
    offset_(off), size_(size)
  {}

  void commit(boost::uint32_t n);
  
  void rollback(boost::uint32_t n);

  raw_region_t buffer();
  
  void mode(mmstore::mode_t m);

  mmstore::mode_t mode() const;

  boost::uint32_t committed() const;
  
  bool is_mapped() const;

private:
  mmstore::mode_t mode_;
  boost::uint32_t committed_;
  boost::uint64_t offset_;
  boost::uint32_t size_;
};



#endif

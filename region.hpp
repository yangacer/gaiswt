#ifndef GAISWT_REGION_HPP_
#define GAISWT_REGION_HPP_

#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include "mmstore.hpp"
#include <utility>
#include <boost/cstdint.hpp>


struct region_impl_t 
: boost::interprocess::mapped_region
{
  typedef boost::interprocess::mapped_region base_t;
  typedef std::pair<void*, boost::uint32_t> raw_region_t;

  region_impl_t();

  region_impl_t(
    boost::interprocess::file_mapping const& mem, 
    mmstore::mode_t mode, 
    boost::int64_t off = 0, 
    boost::uint32_t size = 0,
    void* const addr = 0 );
    
  ~region_impl_t();

  void commit(boost::uint32_t n);
  void rollback(boost::uint32_t n);
  raw_region_t buffer();
  void mode(mmstore::mode_t m);
  void map();
  void unmap();

  mmstore::mode_t mode() const;
  boost::uint32_t committed() const;
  bool is_mapped() const;
  boost::uint32_t get_size() const;

private:
  boost::interprocess::file_mapping const* file_;
  mmstore::mode_t mode_;
  boost::uint32_t committed_;
  boost::int64_t offset_;
  boost::uint32_t size_;
};



#endif

#ifndef GAISWT_MMSTORE_HPP_
#define GAISWT_MMSTORE_HPP_

#include <string>
#include <utility>
#include <set>
#include <list>
#include <boost/cstdint.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/system/error_code.hpp>

struct region_impl_t;
struct map_ele_t;
struct task_t;

struct mmstore : boost::noncopyable
{
  typedef boost::function<void(boost::system::error_code)> 
    completion_handler_t;
  
  enum mode_t { read = 0, write};

  struct region
  {
    friend struct mmstore;

    typedef std::pair<void*, boost::uint32_t> 
      raw_region_t;
    
    region();

    raw_region_t buffer();
    void commit(boost::uint32_t n);

    boost::uint32_t size() const;
    boost::uint64_t offset() const;

    long shared_count() const;
    operator void*() const;

  private:
    region(boost::shared_ptr<region_impl_t> impl);
    boost::shared_ptr<region_impl_t> impl_;
  };

  explicit mmstore(
    std::string dir, 
    std::string maximum_memory = "268435456", // 256mb 
    std::string concurrency_level = "512");
  
  //~mmstore();

  void create( std::string const &name);
  
  region write_region(
    std::string const &name, boost::uint64_t offset = 0);
  
  void async_get_region(
    region &r, 
    std::string const& file, 
    mode_t mode,
    boost::uint64_t offset, 
    completion_handler_t handler);

  // region read_region(
  //  std::string const &name, boost::uint64_t offset = 0);

  boost::uint64_t maximum_memory() const;
  boost::uint32_t maximum_region_size() const;
  boost::uint64_t current_used_memory() const;
  boost::uint64_t available_memory() const;

private:

  boost::unordered_map<
      std::string,
      boost::shared_ptr<map_ele_t>
    > storage_;
  
  std::list<boost::shared_ptr<task_t> > pending_task_;

  std::string prefix_;

  boost::uint64_t 
    maximum_memory_,
    current_used_memory_;
  boost::uint32_t concurrency_level_;
};


#endif // header guard

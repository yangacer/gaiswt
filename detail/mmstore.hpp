#ifndef GAISWT_MMSTORE_HPP_
#define GAISWT_MMSTORE_HPP_

#include <string>
#include <utility>
#include <map>
#include <list>
#include <boost/cstdint.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/interprocess/interprocess_fwd.hpp>
#include <boost/system/error_code.hpp>

namespace detail {

struct region_impl_t;
struct map_ele_t;

struct mmstore : private boost::noncopyable
{
  typedef boost::function<void(boost::system::error_code const &)> 
    completion_handler_t;
  
  enum mode_t { read = 0, write };

  struct region
  {
    friend struct mmstore;

    typedef std::pair<void*, boost::uint32_t> 
      raw_region_t;

    region();
    ~region();

    raw_region_t buffer();
    
    void commit(boost::uint32_t n);
    void rollback(boost::uint32_t n);

    boost::uint32_t committed() const;
    boost::uint32_t size() const;
    boost::int64_t offset() const;
    long shared_count() const;
    
    operator void* const() const;

  private:
    region(boost::shared_ptr<region_impl_t> impl);
    boost::shared_ptr<region_impl_t> impl_;
  };

  explicit mmstore(
    std::string maximum_memory, // = "268435456" (256mb )
    std::string concurrency_level); //= "512");
  
  ~mmstore();

  void create(std::string const &name);
  void rename(std::string const &new_name, std::string const &origin);
  void stop(std::string const &name);
  void remove(std::string const &name);
  void import(std::string const &name);
  
  boost::system::error_code 
  get_region(
    region &region, 
    std::string const& name, 
    mode_t mode,
    boost::int64_t offset);

  void commit_region(region &r);
  
  void set_max_size(std::string const &name, boost::uint64_t size);

  bool is_in(std::string const &name) const;
  boost::int64_t maximum_memory() const;
  boost::uint32_t maximum_region_size() const;
  boost::int64_t current_used_memory() const;
  boost::int64_t available_memory() const;
  boost::int64_t get_max_size(std::string const& name) const;
  boost::int64_t get_current_size(std::string const &name) const;
  boost::uint32_t page_fault() const;
  std::ostream &dump_use_count(std::ostream &os) const;

protected:
  bool swap_idle(boost::uint32_t size);

private:

  std::map<
      std::string,
      boost::shared_ptr<map_ele_t>
    > storage_;
  
  //std::list<boost::shared_ptr<task_t> > pending_task_;

  boost::int64_t 
    maximum_memory_,
    current_used_memory_;
  boost::uint32_t concurrency_level_;
  boost::uint32_t page_fault_;
};

} // namespace detail

#endif // header guard

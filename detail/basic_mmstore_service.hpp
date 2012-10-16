#ifndef GAISWT_BASIC_MMSTORE_SERVICE_HPP_
#define GAISWT_BASIC_MMSTORE_SERVICE_HPP_

#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/detail/bind_handler.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/system/error_code.hpp>
#include <string>
#include <cassert>
#include "mmstore.hpp"

namespace detail {

template <typename Impl>
class basic_mmstore_service
: public boost::asio::io_service::service
{
public:
  static boost::asio::io_service::id id;
  typedef boost::shared_ptr<Impl> implementation_type;

  explicit basic_mmstore_service(
    boost::asio::io_service &io_service)
    : boost::asio::io_service::service(io_service),
    io_service_(),
    work_(new boost::asio::io_service::work(io_service_)),
    thread_(0)
  {
    // boost::unique_lock<boost::mutex> lock(mutex_);
    if(!thread_.get()){
      thread_.reset(new boost::thread(
          boost::bind(&boost::asio::io_service::run, &io_service_)));
    }

  }

  ~basic_mmstore_service()
  {
    shutdown_service();
  }
  
  void construct(implementation_type &impl)
  {
    impl.reset();
  }

  void create(
    implementation_type &impl, 
    std::string const &maximum_memory,
    std::string const &concurrency_level)
  {
    impl.reset(new Impl(maximum_memory, concurrency_level));
  }

  void destroy(implementation_type &impl)
  {
    impl.reset();
  }

  // DEF_INDIRECT_CALL_X
  // X: Number of parameter 
  // R: Reture value type
  // N: Function Name
  // Tn: Parameter type
#define DEF_INDIRECT_CALL_0(R, N) \
  R N(implementation_type &impl){ return impl->N(); }

#define DEF_INDIRECT_CALL_1(R, N, T0) \
  R N(implementation_type &impl, T0 arg0){ return impl->N(arg0); }

#define DEF_INDIRECT_CALL_2(R, N, T0, T1) \
  R N(implementation_type &impl, T0 arg0, T1 arg1){ return impl->N(arg0,arg1); }

#define DEF_INDIRECT_CALL_0_CONST(R, N) \
  R N(implementation_type const &impl) const { return impl->N(); }

#define DEF_INDIRECT_CALL_1_CONST(R, N, T0) \
  R N(implementation_type const &impl, T0 arg0) const { return impl->N(arg0); }

#define DEF_INDIRECT_CALL_2_CONST(R, N, T0, T1) \
  R N(implementation_type const &impl, T0 arg0, T1, arg1) const { return impl->N(arg0,arg1); }

  DEF_INDIRECT_CALL_1(void, create, std::string const&);
  DEF_INDIRECT_CALL_1(void, stop, std::string const&);
  DEF_INDIRECT_CALL_1(void, remove, std::string const&);
  DEF_INDIRECT_CALL_1(void, import, std::string const&);
  DEF_INDIRECT_CALL_1(void, commit_region, mmstore::region &);

  DEF_INDIRECT_CALL_2(void, rename, std::string const&, std::string const&);
  DEF_INDIRECT_CALL_2(void, set_max_size, std::string const&, boost::uint64_t);
  
  DEF_INDIRECT_CALL_1_CONST(bool, is_in, std::string const&);

  DEF_INDIRECT_CALL_0_CONST(boost::int64_t, maximum_memory);
  DEF_INDIRECT_CALL_0_CONST(boost::uint32_t, maximum_region_size);
  DEF_INDIRECT_CALL_0_CONST(boost::int64_t, current_used_memory);
  DEF_INDIRECT_CALL_0_CONST(boost::int64_t, available_memory);
  DEF_INDIRECT_CALL_1_CONST(boost::int64_t, get_max_size, std::string const&);
  DEF_INDIRECT_CALL_1_CONST(boost::int64_t, get_current_size, std::string const&);
  DEF_INDIRECT_CALL_0_CONST(boost::uint32_t, page_fault);

  DEF_INDIRECT_CALL_1_CONST(std::ostream&, dump_use_count, std::ostream &);

  
  template<typename Handler>
  class operation
  {
  public:
    operation(
      implementation_type &impl, 
      boost::asio::io_service &io_service, 
      mmstore::region &region,
      std::string const &name,
      mmstore::mode_t mode,
      boost::int64_t offset,
      Handler handler)
      : impl_(impl),
        io_service_(io_service),
        work_(io_service),
        region_(region),
        name_(name),
        mode_(mode),
        offset_(offset),
        handler_(handler)
    {}
    
    void operator()() const
    {
      using boost::asio::detail::bind_handler;

      implementation_type impl = impl_.lock();
      if(impl){
        boost::system::error_code err = 
          impl->get_region(region_, name_, mode_, offset_);
        if(boost::system::errc::resource_unavailable_try_again == err){
          io_service_.post(operation<Handler>(impl, io_service_, region_, name_, mode_, offset_, handler_));
        }else{
          io_service_.post(bind_handler(handler_, err));
        }
      }else{
        io_service_.post(
          bind_handler(handler_, boost::asio::error::operation_aborted)
          );
      }
    }
  private:
    boost::weak_ptr<Impl> impl_;
    boost::asio::io_service &io_service_;
    boost::asio::io_service::work work_;
    mmstore::region &region_;
    std::string name_;
    mmstore::mode_t mode_;
    boost::int64_t offset_;
    Handler handler_;
  };

  template<typename Handler>
  void async_get_region(
    implementation_type &impl, 
    mmstore::region &r,
    std::string const &name,
    mmstore::mode_t mode,
    boost::int64_t offset,
    Handler handler)
  {
    assert(0 != impl && "impl is reset");
    io_service_.post(
      operation<Handler>(
        impl, this->get_io_service(),
        r, name, mode, offset, handler
        )
      );
  }

  void shutdown_service()
  {
    work_.reset();
    io_service_.stop();
    if(thread_.get()){
      thread_->join();
      thread_.reset();
    }
    io_service_.reset();
  }
private:
   
  boost::asio::io_service io_service_;
  boost::scoped_ptr<boost::asio::io_service::work> work_;
  boost::scoped_ptr<boost::thread> thread_;
  boost::mutex mutex_;
};

template<typename Impl>
boost::asio::io_service::id basic_mmstore_service<Impl>::id;

} // namespace detail

#endif // header guard

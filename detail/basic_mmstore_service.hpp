#ifndef GAISWT_BASIC_MMSTORE_SERVICE_HPP_
#define GAISWT_BASIC_MMSTORE_SERVICE_HPP_

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/system/error_code.hpp>
#include <string>
#include <cassert>
#include "mmstore.hpp"

#include <iostream>

#define get_service_impl(X) boost::asio::use_service<boost::asio::detail::io_service_impl>(X)

namespace ba = boost::asio;
namespace bad = boost::asio::detail;

namespace detail {

class basic_mmstore_service
: public boost::asio::io_service::service
{
public:
  static boost::asio::io_service::id id;

  typedef bad::socket_ops::shared_cancel_token_type implementation_type;
    
  explicit basic_mmstore_service(
    boost::asio::io_service &io_service)
    : ba::io_service::service(io_service),
      io_service_impl_(get_service_impl(io_service) ),
      work_io_service_( new boost::asio::io_service ),
      work_io_service_impl_(get_service_impl(*work_io_service_)),
      work_(new ba::io_service::work(*work_io_service_)),
      work_thread_()
  {}

  ~basic_mmstore_service()
  {
    shutdown_service();
  }
  
  void shutdown_service()
  {
    work_.reset();
    if(work_io_service_.get()){
      work_io_service_->stop();
      if (work_thread_.get()){
        try{
          work_thread_->join();
        }catch(...){
          std::cerr << "[warn] Thread is interruptted\n";
        }
        work_thread_.reset();
      }
    }
    work_io_service_.reset();
  }

  void fork_service(ba::io_service::fork_event fork_ev)
  {
    if (work_thread_.get())
    {
      if (fork_ev == boost::asio::io_service::fork_prepare)
      {
        work_io_service_->stop();
        work_thread_->join();
      }
      else
      {
        work_io_service_->reset();
        work_thread_.reset(new boost::asio::detail::thread(
            work_io_service_runner(*work_io_service_)));
      }
    }
  }

  void construct(implementation_type& impl)
  { 
    BOOST_ASIO_HANDLER_OPERATION(("mmstore", &impl, "construct"));
    impl.reset((void*)0, bad::socket_ops::noop_deleter());
  }

  void create(
    implementation_type &impl,
    std::string const &maximum_memory,
    std::string const &concurrency_level,
    std::string const &meta_file)
  {
    BOOST_ASIO_HANDLER_OPERATION(("mmstore", &impl, "create"));
    impl.reset(new mmstore(maximum_memory, concurrency_level, meta_file));
  }

  void destroy(implementation_type &impl)
  {
    BOOST_ASIO_HANDLER_OPERATION(("mmstore", &impl, "destroy"));
    impl.reset();
  }

  void cancel(implementation_type &impl)
  {
    BOOST_ASIO_HANDLER_OPERATION(("mmstore", &impl, "cancel"));
    impl.reset((void*)0, bad::socket_ops::noop_deleter());
  }

  // DEF_INDIRECT_CALL_X
  // X: Number of parameter 
  // R: Reture value type
  // N: Function Name
  // Tn: Parameter type
#define DEF_INDIRECT_CALL_0(R, N) \
  R N(implementation_type &impl){ return static_cast<mmstore*>(impl.get())->N(); }

#define DEF_INDIRECT_CALL_1(R, N, T0) \
  R N(implementation_type &impl, T0 arg0){ return static_cast<mmstore*>(impl.get())->N(arg0); }

#define DEF_INDIRECT_CALL_2(R, N, T0, T1) \
  R N(implementation_type &impl, T0 arg0, T1 arg1){ return static_cast<mmstore*>(impl.get())->N(arg0,arg1); }

#define DEF_INDIRECT_CALL_0_CONST(R, N) \
  R N(implementation_type const &impl) const { return static_cast<mmstore*>(impl.get())->N(); }

#define DEF_INDIRECT_CALL_1_CONST(R, N, T0) \
  R N(implementation_type const &impl, T0 arg0) const { return static_cast<mmstore*>(impl.get())->N(arg0); }

#define DEF_INDIRECT_CALL_2_CONST(R, N, T0, T1) \
  R N(implementation_type const &impl, T0 arg0, T1, arg1) const { return static_cast<mmstore*>(impl.get())->N(arg0,arg1); }

  DEF_INDIRECT_CALL_1(void, create, std::string const&);
  DEF_INDIRECT_CALL_1(void, stop, std::string const&);
  DEF_INDIRECT_CALL_1(void, remove, std::string const&);
  DEF_INDIRECT_CALL_1(void, import, std::string const&);
  DEF_INDIRECT_CALL_1(void, commit_region, mmstore::region &);
  DEF_INDIRECT_CALL_1(void, serialize, std::ostream&);
  DEF_INDIRECT_CALL_1(void, deserialize, std::istream&);

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
  class get_region_op 
  : public boost::asio::detail::operation
  {
  public:
    BOOST_ASIO_DEFINE_HANDLER_PTR(get_region_op);

    get_region_op(
      bad::socket_ops::weak_cancel_token_type cancel_token,
      mmstore::region &region,
      std::string const &name,
      mmstore::mode_t mode,
      boost::int64_t offset,
      bad::io_service_impl &ios, 
      Handler handler)
      : bad::operation(&get_region_op::do_complete),
        cancel_token_(cancel_token),
        region_(region),
        name_(name),
        mode_(mode),
        offset_(offset),
        io_service_impl_(ios),
        handler_(handler)
    {}
    
    static void do_complete(
      bad::io_service_impl *owner, 
      bad::operation *base,
      boost::system::error_code const &,
      std::size_t )
    {
      get_region_op *o(static_cast<get_region_op*>(base));
      ptr p = { boost::addressof(o->handler_), o, o};
      if(owner && owner != &o->io_service_impl_)
      {
        bad::socket_ops::shared_cancel_token_type lock =
          o->cancel_token_.lock();
        if(!lock){
          o->ec_ = boost::system::error_code(
            ba::error::operation_aborted, boost::system::system_category());
        }else{
          mmstore *impl = static_cast<mmstore*>(lock.get());
          o->ec_ = impl->get_region(
            o->region_, o->name_, o->mode_, o->offset_);
        }
        o->io_service_impl_.post_deferred_completion(o);
        p.v = p.p = 0;
      }else{
        BOOST_ASIO_HANDLER_COMPLETION((o));
        bad::binder1<Handler, boost::system::error_code> 
          handler(o->handler_, o->ec_);
        p.h = boost::addressof(handler.handler_);
        p.reset();
        if(owner){
          bad::fenced_block b(bad::fenced_block::half);
          BOOST_ASIO_HANDLER_INVOCATION_BEGIN((handler.arg1_, "..."));
          boost_asio_handler_invoke_helpers::invoke(handler, handler.handler_);
          BOOST_ASIO_HANDLER_INVOCATION_END;
        }
      }
    }

  private:
    bad::socket_ops::weak_cancel_token_type cancel_token_;
    mmstore::region &region_;
    std::string name_;
    mmstore::mode_t mode_;
    boost::int64_t offset_;
    bad::io_service_impl &io_service_impl_;
    Handler handler_;
    boost::system::error_code ec_;
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
    typedef get_region_op<Handler> op;

    typename op::ptr p = {
      boost::addressof(handler),
      boost_asio_handler_alloc_helpers::allocate(
        sizeof(op), handler), 0};
    
    p.p = new (p.v) op(
      impl, r, name, mode, offset,
      io_service_impl_, handler);

    BOOST_ASIO_HANDLER_CREATION((p.p, "mmstore", &impl, "async_get_region"));
    
    start_op(p.p);

    p.v = p.p = 0;
  }

protected:
  bad::io_service_impl& io_service_impl_;

  class work_io_service_runner
  {
  public:
    work_io_service_runner(boost::asio::io_service &io_service)
      : io_service_(io_service) {}
    void operator()() { 
      // std::cerr << "inner runner started\n";
      io_service_.run(); 
      // std::cerr << "inner runner stopped\n";
    }
  private:
    boost::asio::io_service &io_service_;
  };

  void start_op(bad::operation* op)
  {
    start_work_thread();
    io_service_impl_.work_started();
    work_io_service_impl_.post_immediate_completion(op);
  }

  void start_work_thread()
  {
    bad::mutex::scoped_lock lock(mutex_);
    if (!work_thread_.get())
    {
      work_thread_.reset(new bad::thread(
          work_io_service_runner(*work_io_service_)));
    }
  }

private:
   
  bad::mutex mutex_;
  boost::scoped_ptr<ba::io_service> work_io_service_;
  bad::io_service_impl &work_io_service_impl_;
  boost::scoped_ptr<ba::io_service::work> work_;
  boost::scoped_ptr<bad::thread> work_thread_;
};


} // namespace detail

#endif // header guard

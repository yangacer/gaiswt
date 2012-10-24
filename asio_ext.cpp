#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <cerrno>

#define get_service_impl(X) ba::use_service<bad::io_service_impl>(X)

namespace ba = boost::asio;
namespace bad = boost::asio::detail;

struct logger_impl
{
  explicit logger_impl(const std::string& ident, std::string const& file) 
  : identifier_(ident),  
    ofstream_(file.c_str(), std::ios::app | std::ios::out)
  {
    if(!ofstream_.is_open())
      throw std::runtime_error("logger_impl: open file failed");
  }

  ~logger_impl()
  { ofstream_.close(); }

  boost::system::error_code log(std::string const& msg)
  { 
    errno = 0;
    ofstream_ << identifier_ << ": " << msg << "\n"; 
    return 
      boost::system::error_code(
        errno, 
        boost::system::system_category());
  }
private:
  std::string identifier_;
  std::ofstream ofstream_;
};

template<typename Handler>
class log_op : public bad::operation
{
public:
  BOOST_ASIO_DEFINE_HANDLER_PTR(log_op);
  
  log_op(
    bad::socket_ops::weak_cancel_token_type cancel_token,
    std::string const& msg,
    bad::io_service_impl& ios, Handler& handler)
    : bad::operation(&log_op::do_complete),
    cancel_token_(cancel_token),
    msg_(msg),
    io_service_impl_(ios),
    handler_(handler)
  {}

  ~log_op()
  {}

  static void do_complete(
    bad::io_service_impl *owner, 
    bad::operation *base,
    boost::system::error_code const &,
    std::size_t )
  {
    log_op *o(static_cast<log_op*>(base));
    ptr p = { boost::addressof(o->handler_), o, o};
    if(owner && owner != &o->io_service_impl_)
    {
      bad::socket_ops::shared_cancel_token_type lock =
        o->cancel_token_.lock();
      if(!lock){
        o->ec_ = boost::system::error_code(
          ba::error::operation_aborted, boost::system::system_category());
      }else{
        logger_impl *impl = static_cast<logger_impl*>(lock.get());
        o->ec_ = impl->log(o->msg_);
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
  std::string msg_;
  bad::io_service_impl &io_service_impl_;
  Handler handler_;
  boost::system::error_code ec_;
};

class logger_service
: public ba::io_service::service
{
public:
  // Actually this is of type shared_ptr<void> 
  typedef bad::socket_ops::shared_cancel_token_type implementation_type;
  
  // XXX
  static boost::asio::io_service::id id;

  logger_service(ba::io_service& io_service)
    : ba::io_service::service(io_service),
      io_service_impl_(get_service_impl(io_service) ),
      work_io_service_( new boost::asio::io_service ),
      work_io_service_impl_(get_service_impl(*work_io_service_)),
      work_(new ba::io_service::work(*work_io_service_)),
      work_thread_()
  {}

  ~logger_service()
  {
    shutdown_service();
  }

  void shutdown_service()
  {
    work_.reset();
    if(work_io_service_.get()){
      work_io_service_->stop();
      if (work_thread_.get()){
        work_thread_->join();
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
    impl.reset((void*)0, bad::socket_ops::noop_deleter());
  }

  void create(
    implementation_type& impl, 
    const std::string &identifier,
    const std::string &file)
  {
    impl.reset(new logger_impl(identifier, file));
  }

  void destroy(implementation_type& impl)
  {
    BOOST_ASIO_HANDLER_OPERATION(("logger", &impl, "destroy"));
    impl.reset();
  }

  void cancel(implementation_type& impl)
  {
    BOOST_ASIO_HANDLER_OPERATION(("logger", &impl, "cancel"));
    
    impl.reset((void*)0, bad::socket_ops::noop_deleter());
  }

  template<typename Handler>
  void async_log(
    implementation_type& impl, 
    std::string const& msg, 
    Handler handler)
  {
    typedef log_op<Handler> op;
    
    typename op::ptr p = {
      boost::addressof(handler),
      boost_asio_handler_alloc_helpers::allocate(
        sizeof(op), handler), 0};

    p.p = new (p.v) op(impl, msg, io_service_impl_, handler);
    
    BOOST_ASIO_HANDLER_CREATION((p.p, "logger", &impl, "async_log"));
    
    start_op(p.p);
    p.v = p.p = 0;
  }

protected:

  class work_io_service_runner
  {
  public:
    work_io_service_runner(ba::io_service& io_service)
      : io_service_(io_service){}
    void operator()() { io_service_.run(); }
  private:
    ba::io_service &io_service_;
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

  bad::io_service_impl& io_service_impl_;

private:

  bad::mutex mutex_;
  boost::scoped_ptr<ba::io_service> work_io_service_;

  bad::io_service_impl &work_io_service_impl_;
  boost::scoped_ptr<ba::io_service::work> work_;
  boost::scoped_ptr<bad::thread> work_thread_;
  std::ofstream ofstream_;
};

boost::asio::io_service::id logger_service::id;


template <typename Service>
class basic_logger
: public ba::basic_io_object<Service>
{
public:

  explicit basic_logger(
    ba::io_service& io_service,
    const std::string &identifier,
    const std::string &file)
    :ba::basic_io_object<Service>(io_service)
  { 
    this->service.create(this->implementation, identifier, file);  
  }

  void cancel()
  {
    return this->service.cancel(this->implementation);
  }

  template<typename Handler>
  void async_log(
    std::string const& msg, 
    Handler handler)
  {
    this->service.async_log(this->implementation, msg, handler);
  }
};

typedef basic_logger<logger_service> logger;


void callback(boost::system::error_code const &ec)
{}

int main()
{
  {
    boost::asio::io_service io_service;
    //boost::asio::signal_set signals(io_service);
    //signals.add(SIGINT);
    //signals.async_wait(boost::bind(&boost::asio::io_service::stop, &io_service));

    logger log(io_service, "test", "logger.log");

    log.async_log("msg1", boost::bind(&callback, _1));
    log.async_log("msg2", boost::bind(&callback, _1));

    io_service.run();
  }
  
  {
    boost::asio::io_service io_service;
    //boost::asio::signal_set signals(io_service);
    //signals.add(SIGINT);
    //signals.async_wait(boost::bind(&boost::asio::io_service::stop, &io_service));

    logger log(io_service, "test", "logger.log");

    log.async_log("msg3", boost::bind(&callback, _1));
    log.async_log("msg4", boost::bind(&callback, _1));

    io_service.run();
  }

  return 0;
}

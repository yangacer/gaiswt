#include "mmstore.hpp"
#include "basic_mmstore_service.hpp"
#include "basic_mmstore.hpp"
#include <iostream>
#include <boost/bind.hpp>

template<typename Impl>
boost::asio::io_service::id basic_mmstore_service<Impl>::id;

namespace experiment {
  typedef basic_mmstore< 
    basic_mmstore_service<
    mmstore
    > 
    > new_mmstore;
  typedef mmstore::region region;
}

struct writer
{
  writer(experiment::new_mmstore &mms)
    : mms_(mms)
  {
    std::cerr << "writer starts\n";
    start_write();
  }

  void start_write()
  {
    mms_.async_get_region(
      region_, "tmp.file", mmstore::write, 0, 
      boost::bind(&writer::handle_region, this, _1));
    std::cerr << "writer async called\n";
  }

  void handle_region(boost::system::error_code const & err)
  {
    if(!err){
      experiment::region::raw_region_t buf =
        region_.buffer();
      memcpy(buf.first, "testing\n123\n", 13);
      std::cerr << "writer is done\n";
      region_.commit(13);
      mms_.commit_region(region_);
    }else{
      std::cerr << "writer error: " << err.message() << "\n";
    }
  }

  experiment::new_mmstore &mms_;
  experiment::region region_;
};

struct reader
{
  reader(experiment::new_mmstore &mms)
    : mms_(mms)
  {
    std::cerr << "reader starts\n";
    start_read();
  }
  
  void start_read()
  {
    mms_.async_get_region(
      region_, "tmp.file", mmstore::read, 0,
      boost::bind(&reader::handle_region, this, _1));
    std::cerr << "reader async called\n";
  }

  void handle_region(boost::system::error_code const &err)
  {
    if(!err){
      experiment::region::raw_region_t buf =
        region_.buffer();
      std::cerr << "reader read: " << (char const*)buf.first << "\n";
      mms_.commit_region(region_);
    }else{
      std::cerr << "reader error: " << err.message() << "\n";
    }
  }

  experiment::new_mmstore &mms_;
  experiment::region region_;
};

int main()
{
  boost::asio::io_service io_service;
  experiment::new_mmstore mms(io_service, "10240", "2");
  mms.create("tmp.file"); 

  reader r(mms);
  writer w(mms);
  
  io_service.run();
  return 0;
}

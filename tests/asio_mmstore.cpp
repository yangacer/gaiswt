#include "mmstore.hpp"
#include <iostream>
#include <boost/bind.hpp>

struct writer
{
  writer(mmstore &mms)
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
      mmstore::region::raw_region_t buf =
        region_.buffer();
      memcpy(buf.first, "testing\n123\n", 13);
      std::cerr << "writer is done\n";
      region_.commit(13);
      mms_.commit_region(region_);
    }else{
      std::cerr << "writer error: " << err.message() << "\n";
    }
  }

  mmstore &mms_;
  mmstore::region region_;
};

struct reader
{
  reader(mmstore &mms)
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
      mmstore::region::raw_region_t buf =
        region_.buffer();
      std::cerr << "reader read: " << (char const*)buf.first << "\n";
      mms_.commit_region(region_);
    }else{
      std::cerr << "reader error: " << err.message() << "\n";
    }
  }

  mmstore &mms_;
  mmstore::region region_;
};

int main()
{
  boost::asio::io_service io_service;
  mmstore mms(io_service, "10240", "2", "asio_mmstore.mms");
  mms.create("tmp.file"); 

  reader r(mms);
  writer w(mms);
  
  io_service.run();
  return 0;
}

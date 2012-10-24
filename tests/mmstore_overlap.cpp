#include "mmstore.hpp"
#include <boost/system/error_code.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <cassert>

struct get_1024
{
  get_1024(mmstore &mms)
  {
    mms.async_get_region(
      r, "overlap.tmp", mmstore::write, 1024,
      boost::bind(&get_1024::handle_region, this, _1));
  }
  
  void handle_region(boost::system::error_code const &err)
  {
    std::cout << r.offset() << ", " << r.size() << "\n";
    assert(r.offset() == 1024 && r.size() == 4096 && "get region overlapped");
  }

  mmstore::region r;
};

struct get_0
{
  get_0(mmstore &mms)
  {
    mms.async_get_region(
      r, "overlap.tmp", mmstore::write, 0,
      boost::bind(&get_0::handle_region, this, _1));
  }

  void handle_region(boost::system::error_code const &err)
  {
    std::cout << r.offset() << ", " << r.size() << "\n";    
    assert(r.offset() == 0 && r.size() == 1024 && "get region overlapped");
  }

  mmstore::region r;
};

int main()
{
  boost::asio::io_service io_service;
  mmstore mms(io_service, "8192", "2", "mmstore_overlap.mms");
  
  mms.create("overlap.tmp");

  get_1024 g1(mms);
  get_0 g2(mms);
  
  io_service.run();
  return 0;
}

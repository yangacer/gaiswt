#include "mmstore.hpp"
#include <boost/asio/signal_set.hpp>
#include <fstream>

struct writer 
{
  writer(mmstore &mms)
    : mms_(mms)
  {
    mms.async_get_region(
      region,
      "serialize_test", 
      mmstore::write,
      0,
      boost::bind(&writer::handle_write, this,_1));
  }

  void handle_write(boost::system::error_code const& err)
  {
    if(!err){
      mmstore::region::raw_region_t buf(region.buffer());
      memcpy(buf.first, "testing\n123\n", 13);
      region.commit(13);
      mms_.commit_region(region);
    }
  }
  mmstore &mms_;
  mmstore::region region;
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
      region_, "serialize_test", mmstore::read, 0,
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
  {
    boost::asio::io_service io_service;

    remove("mmstore_serialize.mms");

    mmstore mms(io_service, "10240", "2", "mmstore_serialize.mms");
    mms.create("serialize_test");
    writer w(mms);

    io_service.run(); // trigger serialization
  }

  {
    boost::asio::io_service io_service;
    mmstore mms(io_service, "10240", "2", "mmstore_serialize.mms");
    reader r(mms);

    io_service.run(); // trigger deserialization
  }

  return 0;
}

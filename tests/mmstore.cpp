#include "mmstore.hpp"
#include <boost/asio.hpp>
#include <boost/ref.hpp>
#include <boost/shared_array.hpp>
#include <iostream>
#include <exception>
#include "coroutine.hpp"

using boost::shared_ptr;

struct writer : coroutine
{

  typedef boost::uint32_t uint32_t;
  
  writer(mmstore &mms)
    : region(new mmstore::region),
      mms_(mms),
      size(new uint32_t),
      to_cpy(new uint32_t), offset(new uint32_t)
  {
    *size = (mms_.maximum_region_size()<<1) + 1024;
    fake.reset(new char[*size+1]);
    // fake some data
    for(uint32_t i=0;i<*size; ++i)
      fake[i] = 'a' + i % 26;
  }
  
#include "yield.hpp"
  void operator()(boost::system::error_code err = boost::system::error_code())
  {
    // XXX Buggy
    reenter(this){
      while(*offset < *size){
        //region.reset(new mmstore::region);
        yield mms_.async_get_region(
          *region, "test1.file", mmstore::write, 
          *offset, *this);
        mmstore::region::raw_region_t buf = 
          region->buffer();
        *to_cpy = std::min(buf.second, *size - *offset);
        memcpy(buf.first, fake.get() + *offset , *to_cpy);
        *offset += *to_cpy;
        region->commit(*to_cpy);
        mms_.commit_region(*region, "test1.file");
      }
    }
  }
#include "unyield.hpp"

  shared_ptr<mmstore::region> region;
  mmstore &mms_;

  boost::shared_array<char> fake;
  shared_ptr<uint32_t> size, to_cpy, offset;
};

struct reader : coroutine
{
  reader(mmstore &mms)
    : mms_(mms)
  {}
#include "yield.hpp"
  void operator()(boost::system::error_code err)
  {
    reenter(this){
      yield mms_.async_get_region(region, "test1.file", mmstore::read, 0, *this);
      std::cout << 
        "offset: " << region.buffer().first <<
        "size: " << region.buffer().second << "\n";
    }
  }
#include "unyield.hpp"
  mmstore &mms_;
  mmstore::region region;
};

int main(int argc, char** argv)
{
  try{
    if(argc != 4){
      std::cout << 
        "Usage: mmstore <directory> <max_memory> <concurrent_level>\n" ;
      std::cout << "\te.g. mmstore . 10240 2\n" ;
      return 1;
    }

    mmstore mms(argv[1], argv[2], argv[3]);
    
    std::cout << "max_memory: " << mms.maximum_memory() << "\n";
    std::cout << "maximum_region_size: " << mms.maximum_region_size() << "\n";
    
    mms.create("test1.file");

    writer wrt(mms);
    wrt();

  }catch(std::exception &e){
    std::cout << "Exception: " << e.what() << "\n";
  }
}

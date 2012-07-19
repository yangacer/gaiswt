#include "mmstore.hpp"
#include <boost/asio.hpp>
#include <boost/ref.hpp>
#include <iostream>
#include <exception>
#include "coroutine.hpp"

struct writer : coroutine
{
  writer(mmstore &mms)
    : mms_(mms)
  {}

#include "yield.hpp"
  void operator()(boost::system::error_code err = boost::system::error_code())
  {
    reenter(this){
      //region.reset();
      yield mms_.async_get_region(region, "test1.file", mmstore::write, 0, *this);
      //mmstore::region::raw_region_t buf = region.buffer();
      
      // fake some data
      //char *fake = new char[buf.second];
      //for(int i=0;i<buf.second; ++i)
      //  fake[i] = 'a' + i % 26;
      //memcpy(buf.first, fake, buf.second);
      //buf.commit(buf.second);
    }
  }
#include "unyield.hpp"

  mmstore::region region;
  mmstore &mms_;
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

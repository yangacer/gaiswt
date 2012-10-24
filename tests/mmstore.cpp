#include "mmstore.hpp"
#include <boost/asio.hpp>
#include <boost/ref.hpp>
#include <boost/shared_array.hpp>
#include <iostream>
#include <exception>
#include "coroutine.hpp"
#include <boost/bind.hpp>

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
  
  ~writer()
  { 
    //std::cout << "writer is destroied\n" <<
    //  "written size: " << *offset << "\n"; 
  }
#include "yield.hpp"
  void operator()(boost::system::error_code err = boost::system::error_code())
  {
    if(!err){
      reenter(this){
        while(*offset < *size){
          
          yield mms_.async_get_region(
            *region, "test1.file", mmstore::write, 
            *offset, *this);

          mmstore::region::raw_region_t buf = 
            region->buffer();

          if(!buf.second)
            break;

          *to_cpy = std::min(buf.second, *size - *offset);
          memcpy(buf.first, fake.get() + *offset , *to_cpy);
          *offset += *to_cpy;
          region->commit(*to_cpy);
          mms_.commit_region(*region);
        }
      }
    }else{
      std::cout << err.message() << "\n";
    }
  }
#include "unyield.hpp"

  shared_ptr<mmstore::region> region;
  mmstore &mms_;

  boost::shared_array<char> fake;
  shared_ptr<uint32_t> size, to_cpy, offset;
};

struct writer2
{
  writer2(mmstore &mms)
    : mms_(mms), to_cpy(0), offset(0)
  {
    size = (mms_.maximum_region_size()<<1) + 1024;
    fake.reset(new char[size+1]);
    // fake some data
    for(uint32_t i=0;i<size; ++i)
      fake[i] = 'a' + i % 26;

    mms_.async_get_region(
      region, "test1.file",
      mmstore::write,
      offset,
      boost::bind(
        &writer2::handle_region, this, _1
        )
      );
  }
  
  ~writer2()
  { 
    //std::cout << "writer2 is destroied\n" 
    //  << "written size: " << offset << std::endl;
  }

  void handle_region(
    boost::system::error_code err = boost::system::error_code() )
  {
    if(!err){
      mmstore::region::raw_region_t buf = 
        region.buffer();
      to_cpy = std::min(buf.second, size - offset);
      memcpy(buf.first, fake.get() + offset , to_cpy);
      offset += to_cpy;
      region.commit(to_cpy);
      mms_.commit_region(region);

      if(offset < size){
        mms_.async_get_region(
          region, "test1.file",
          mmstore::write,
          offset,
          boost::bind(
            &writer2::handle_region, this, _1
            )
          );
      }
    }else{
      std::cout << err.message() << "\n";
    }
  }

  mmstore::region region;
  mmstore &mms_;
  boost::shared_array<char> fake;
  uint32_t size, to_cpy, offset;
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
    if(argc != 3){
      std::cout << 
        "Usage: mmstore <max_memory> <concurrent_level>\n" ;
      std::cout << "\te.g. mmstore 10240 2\n" ;
      return 1;
    }

    boost::asio::io_service io_service;
    mmstore mms(io_service, argv[1], argv[2], "mmstore.mms");
    
    std::cout << "max_memory: " << mms.maximum_memory() << "\n";
    std::cout << "maximum_region_size: " << mms.maximum_region_size() << "\n";
    
    mms.create("test1.file");
    mms.create("test2.file");

    // invoke writer2
    writer2 wrt2(mms);
    
    mms.dump_use_count(std::cout);

    std::cout << "\nCurrent size of test1.file: " << 
      mms.get_current_size("test1.file") << "\n";
    std::cout << "Page fault: " << mms.page_fault() << "\n";


    // invoke writer
    {
      writer wrt(mms);
      wrt();
    }

    io_service.run();

    mms.dump_use_count(std::cout);
    std::cout << "\nCurrent size of test1.file: " << 
      mms.get_current_size("test1.file") << "\n";
    std::cout << "Page fault: " << mms.page_fault() << "\n";

  }catch(std::exception &e){
    std::cout << "Exception: " << e.what() << "\n";
  }
}

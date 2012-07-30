#include "coroutine.hpp"
#include "mmstore.hpp"
#include "agent.hpp"
#include <boost/system/error_code.hpp>
#include <boost/shared_ptr.hpp>

using boost::system::error_code;
using boost::shared_ptr;
namespace asio = boost::asio;

struct fetcher : coroutine
{
  struct ctx;

  fetcher(asio::io_service &io_service, 
          mmstore &mms, 
          std::string const &link)
  : mms_(mms), page_(io_service), cdn_(io_service), ctx_()
  {
    ctx_.reset(new ctx);
    ctx_->page_lnk = link;
  }

  void operator()(error_code ec = error_code())
  {
    reenter(this){
      
      yield page_.get(ctx_->page_lnk, *this);
      if(ec){
        std::cout << "fetch page failed\n";
        return;
      }

      // TODO get cdn link and determine filename
      
      do{
        yield mms_.async_get_region(
          ctx_->region, ctx_->vid_id, 
          mmstore::write,
          ctx_->offset, *this);

        yield cdn_.get_some(
          ctx_->cdn_lnk, ctx_->region, *this);

        if(ec && ec != asio::error::eof)
          break;

        ctx_->offset += ctx_->region->committed();
        mms_.commit_region(ctx_->region, ctx_->vid_id_);

        if(ec == asio::error::eof)
          break;
      }while(1);
    }
  }

  typedef asio::ip::tcp tcp;

  mmstore &mms_;
  http::agent page_, cdn_;
  
  struct ctx 
  {
    std::string page_lnk, cdn_lnk, vid_id;
    mmstore::region region;
    boost::uint32_t offset;
  };

  shared_ptr<ctx> ctx_;
};

int main(int argc, char** argv)
{
  try{
    if(argc != 2){
      std::cout << "Usage: tubefetch <video_link>\n";
      return 1;
    }
  }catch(std::exception &e){
    std::cout << e.what() << "\n";
  }
  return 0;
}

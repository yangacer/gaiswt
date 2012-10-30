#include "detail/mmstore.hpp"
#include <boost/program_options.hpp>
#include <iostream>
#include <fstream>
#include <string>

int main(int argc, char **argv)
{
  using namespace std;
  namespace po = boost::program_options;

  po::options_description desc("Options");
  po::variables_map vmap;

  string mms_meta, source;

  desc.add_options()
    ("help", "Help message")
    ("mms", 
      po::value<string>(&mms_meta),
      "File for save/load snapshot of mmstore.")
    ("file",
      po::value<string>(&source),
      "File to be imported to a mmstore.")
    ("verify", "Verify import is correct.")
    ;

  try{
    po::store(po::parse_command_line(argc, argv, desc), vmap);
    po::notify(vmap);
  }catch(po::error &e){
    cerr << desc << "\n";
    return 1;
  }
  
  if(vmap.count("help") || !vmap.count("mms") || !vmap.count("file")){
    cerr << desc << "\n";
    return 1;
  }

  ifstream ifs(mms_meta);
  if(!ifs.is_open()){
    cerr << "mms file open failed\n";
    cerr << desc << "\n";
    return 1;
  }

  {
    detail::mmstore mms("0","0", mms_meta);
    mms.import(source);
  }

  if(vmap.count("verify")){
    detail::mmstore mms("0","0", mms_meta);
    if(!mms.is_in(source)){
      cerr << "Verification of file (" << source << ") failed (not found)\n";
      return 1;
    }
    
    detail::mmstore::region region;
    boost::system::error_code ec;
    boost::uint32_t total(0);
    while(!(ec = mms.get_region(region, source, detail::mmstore::read, total))){
      total += region.committed();
      cerr << ec.message() << "\n";
    }
    // XXX Should be End of file
    cerr << "read status: " << ec.message() << "\n";
    if(mms.get_max_size(source) != total){
      cerr << "Verification of file (" << source << ") failed (" <<
        total << "/" << mms.get_max_size(source) << 
        ")\n";
      return 1;
    }
  }

  return 0;
}

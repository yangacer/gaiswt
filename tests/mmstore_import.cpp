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
      cerr << "Verification of file (" << source << ") failed\n";
      return 1;
    }
    cout << mms.get_max_size(source) << "\n";
  }

  return 0;
}

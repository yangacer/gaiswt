#include "region.hpp"
#include <boost/interprocess/file_mapping.hpp>
#include <boost/shared_ptr.hpp>
#include "mmstore.hpp"

namespace ipc = boost::interprocess;

int main()
{
  using boost::shared_ptr;
  mmstore mms;

  ipc::file_mapping
    file("test1.file", ipc::read_write);

  shared_ptr<region_impl_t> ptr(
    new region_impl_t(mms, file, mmstore::write, 0, 1024)
    );

  memcpy(ptr->get_address(), "test", 4);

  return 0;
}

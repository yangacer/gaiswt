#include "mmstore.hpp"

int main(int argc, char **argv)
{
  mmstore mms;
  mms.import(argv[1]);
  return 0;
}

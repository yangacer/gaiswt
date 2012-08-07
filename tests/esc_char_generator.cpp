#include "generator_def.hpp"

int main()
{
  using namespace http::generator;

  std::string input = "abc[許功蓋]ef", output;
  std::back_insert_iterator<std::string> out(output);
  url_esc_string<decltype(out)> gen;

  if(!karma::generate(out, gen, input))
    std::cout << "Generating failed\n";
  
  std::cout << output << "\n";

  return 0;
}

#define GAISWT_DEBUG_GENERATOR
#include "generator_def.hpp"

int main()
{
  using namespace http;

  http::entity::uri uri;

  std::string output;

  {
    std::back_insert_iterator<std::string> out(output);
    generator::url_esc_string<decltype(out)> esc_string_gen;

    uri.path = "/www.example.com/[acer].html";

    if(!generator::karma::generate(
        out, esc_string_gen(generator::RESERVE_PATH_DELIM), uri.path))
      std::cout << "Generating path failed\n";
  }

  std::cout << output << "\n";

  output.clear();
  {
    std::back_insert_iterator<std::string> out(output);
    generator::uri<decltype(out)> uri_gen;

    entity::query_pair_t key_val;

    key_val.first = "oid"; 
    key_val.second = 123ll; // 123 long long 

    uri.query_map.insert(key_val);

    key_val.first = "name";
    key_val.second = "yangacer";

    uri.query_map.insert(key_val);

    if(!generator::karma::generate(out, uri_gen, uri))
      std::cout << "Generating uri failed\n";

    std::cout << output << "\n";
  }

}

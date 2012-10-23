#ifndef GAISWT_MAP_ELE_HPP_
#define GAISWT_MAP_ELE_HPP_

#include <string>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/interprocess/file_mapping.hpp>
#include <set>
#include <boost/serialization/base_object.hpp>

// ---------- mmstore::map_ele_t impl --------------

namespace detail {

struct region_impl_t;

struct map_ele_t
: private std::set<boost::shared_ptr<region_impl_t> >
{
  typedef std::set<boost::shared_ptr<region_impl_t> > super_type;
  typedef boost::interprocess::file_mapping file_mapping;

  map_ele_t();
  map_ele_t(std::string const& fname);
  
  void open(std::string const& fname);

  template<class Ar>
  void serialize(Ar &ar, unsigned int const)
  {
    ar & boost::serialization::base_object<super_type>(*this);
    ar & max_size_;
  }

  file_mapping mfile;
  boost::int64_t max_size_;
  bool is_open_;

  using super_type::begin;
  using super_type::end;
  using super_type::size;
  using super_type::insert;
  using super_type::erase;
  using super_type::find;
};

}

#endif

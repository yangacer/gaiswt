#ifndef GAISWT_BASIC_MMSTORE_HPP_
#define GAISWT_BASIC_MMSTORE_HPP_

#include <boost/noncopyable.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/basic_io_object.hpp>
#include "basic_mmstore_service.hpp"

namespace detail {

template <typename Service> 
class basic_mmstore 
: public boost::asio::basic_io_object<Service>
{ 
public: 
  typedef typename Service::implementation_type implementation_type;
  typedef typename implementation_type::element_type element_type;
  typedef typename detail::mmstore::region region;
  typedef typename detail::mmstore::mode_t mode_t;
  
  static mode_t const write;
  static mode_t const read;

  explicit basic_mmstore(
    boost::asio::io_service &io_service,
    std::string const &maximum_memory,
    std::string const &concurrency_level,
    std::string const &meta_file)
    : boost::asio::basic_io_object<Service>(io_service)
  {
    this->service.create(this->implementation, maximum_memory, concurrency_level, meta_file);
  } 

  boost::asio::io_service&
  get_io_service()
  { return this->service.get_io_service(); }

  ~basic_mmstore()
  { this->service.destroy(this->implementation); }

#define DEF_INDIRECT_SERVICE_CALL_0(R, N) \
  R N(){ return this->service.N(this->implementation); }

#define DEF_INDIRECT_SERVICE_CALL_1(R, N, T0) \
  R N( T0 arg0){ return this->service.N(this->implementation,arg0); }

#define DEF_INDIRECT_SERVICE_CALL_2(R, N, T0, T1) \
  R N( T0 arg0, T1 arg1){ return this->service.N(this->implementation,arg0,arg1); }

#define DEF_INDIRECT_SERVICE_CALL_0_CONST(R, N) \
  R N() const { return this->service.N(this->implementation); }

#define DEF_INDIRECT_SERVICE_CALL_1_CONST(R, N, T0) \
  R N( T0 arg0) const { return this->service.N(this->implementation,arg0); }

#define DEF_INDIRECT_SERVICE_CALL_2_CONST(R, N, T0, T1) \
  R N( T0 arg0, T1, arg1) const { return this->service.N(this->implementation,arg0, arg1); }

  DEF_INDIRECT_SERVICE_CALL_0(void, cancel);

  DEF_INDIRECT_SERVICE_CALL_1(void, create, std::string const&);
  DEF_INDIRECT_SERVICE_CALL_1(void, stop, std::string const&);
  DEF_INDIRECT_SERVICE_CALL_1(void, remove, std::string const&);
  DEF_INDIRECT_SERVICE_CALL_1(void, import, std::string const&);
  DEF_INDIRECT_SERVICE_CALL_1(void, commit_region, mmstore::region &);
  DEF_INDIRECT_SERVICE_CALL_1(void, serialize, std::ostream &);
  DEF_INDIRECT_SERVICE_CALL_1(void, deserialize, std::istream &);

  DEF_INDIRECT_SERVICE_CALL_2(void, rename, std::string const&, std::string const&);
  DEF_INDIRECT_SERVICE_CALL_2(void, set_max_size, std::string const&, boost::uint64_t);
  
  DEF_INDIRECT_SERVICE_CALL_1_CONST(bool, is_in, std::string const&);

  DEF_INDIRECT_SERVICE_CALL_0_CONST(boost::int64_t, maximum_memory);
  DEF_INDIRECT_SERVICE_CALL_0_CONST(boost::uint32_t, maximum_region_size);
  DEF_INDIRECT_SERVICE_CALL_0_CONST(boost::int64_t, current_used_memory);
  DEF_INDIRECT_SERVICE_CALL_0_CONST(boost::int64_t, available_memory);
  DEF_INDIRECT_SERVICE_CALL_1_CONST(boost::int64_t, get_max_size, std::string const&);
  DEF_INDIRECT_SERVICE_CALL_1_CONST(boost::int64_t, get_current_size, std::string const&);
  DEF_INDIRECT_SERVICE_CALL_0_CONST(boost::uint32_t, page_fault);

  DEF_INDIRECT_SERVICE_CALL_1_CONST(std::ostream&, dump_use_count, std::ostream &);

  template <typename Handler> 
  void async_get_region(
    mmstore::region &r,
    std::string const &name,
    mmstore::mode_t mode,
    boost::int64_t offset,
    Handler handler)
  { 
    this->service.async_get_region(this->implementation, r, name, mode, offset, handler);
  }

};

template<typename T>
typename basic_mmstore<T>::mode_t const basic_mmstore<T>::read(mmstore::read);

template<typename T>
typename basic_mmstore<T>::mode_t const basic_mmstore<T>::write(mmstore::write);

} // namespace detail

#endif // header guard

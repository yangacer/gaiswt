#include "parser_def.hpp"
#include <boost/asio.hpp>

namespace http { namespace parser {
  
  template response_first_line<istream_iterator>::response_first_line();
  template header_list<istream_iterator>::header_list();
  template field<istream_iterator>::field();

  template response_first_line<
    boost::asio::buffers_iterator<
    boost::asio::streambuf::const_buffers_type
    > 
    >::response_first_line();
  template header_list<
    boost::asio::buffers_iterator<
    boost::asio::streambuf::const_buffers_type
    >>::header_list();
  template field<
    boost::asio::buffers_iterator<
    boost::asio::streambuf::const_buffers_type
    >>::field();

  template response_first_line<std::string::iterator>::response_first_line();
  template header_list<std::string::iterator>::header_list();
  template field<std::string::iterator>::field();

}} // namespace http::parser

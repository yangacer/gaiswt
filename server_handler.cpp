#include "server_handler.hpp"
#include "status_code.hpp"
#include "mime_types.hpp"
#include <boost/lexical_cast.hpp>
#include <ostream>
#include <iostream>

namespace http {

namespace stock_replies {

const char ok[] = "";
const char created[] =
  "<html>"
  "<head><title>Created</title></head>"
  "<body><h1>201 Created</h1></body>"
  "</html>";
const char accepted[] =
  "<html>"
  "<head><title>Accepted</title></head>"
  "<body><h1>202 Accepted</h1></body>"
  "</html>";
const char no_content[] =
  "<html>"
  "<head><title>No Content</title></head>"
  "<body><h1>204 Content</h1></body>"
  "</html>";
const char multiple_choices[] =
  "<html>"
  "<head><title>Multiple Choices</title></head>"
  "<body><h1>300 Multiple Choices</h1></body>"
  "</html>";
const char moved_permanently[] =
  "<html>"
  "<head><title>Moved Permanently</title></head>"
  "<body><h1>301 Moved Permanently</h1></body>"
  "</html>";
const char found[] =
  "<html>"
  "<head><title>Moved Temporarily</title></head>"
  "<body><h1>302 Found</h1></body>"
  "</html>";
const char not_modified[] =
  "<html>"
  "<head><title>Not Modified</title></head>"
  "<body><h1>304 Not Modified</h1></body>"
  "</html>";
const char bad_request[] =
  "<html>"
  "<head><title>Bad Request</title></head>"
  "<body><h1>400 Bad Request</h1></body>"
  "</html>";
const char unauthorized[] =
  "<html>"
  "<head><title>Unauthorized</title></head>"
  "<body><h1>401 Unauthorized</h1></body>"
  "</html>";
const char forbidden[] =
  "<html>"
  "<head><title>Forbidden</title></head>"
  "<body><h1>403 Forbidden</h1></body>"
  "</html>";
const char not_found[] =
  "<html>"
  "<head><title>Not Found</title></head>"
  "<body><h1>404 Not Found</h1></body>"
  "</html>";
const char internal_server_error[] =
  "<html>"
  "<head><title>Internal Server Error</title></head>"
  "<body><h1>500 Internal Server Error</h1></body>"
  "</html>";
const char not_implemented[] =
  "<html>"
  "<head><title>Not Implemented</title></head>"
  "<body><h1>501 Not Implemented</h1></body>"
  "</html>";
const char bad_gateway[] =
  "<html>"
  "<head><title>Bad Gateway</title></head>"
  "<body><h1>502 Bad Gateway</h1></body>"
  "</html>";
const char service_unavailable[] =
  "<html>"
  "<head><title>Service Unavailable</title></head>"
  "<body><h1>503 Service Unavailable</h1></body>"
  "</html>";

} // namespace stok_replies

#define CREATE_RESPONSE(Code) \
{ \
  resp_ = response::stock_response(status_type::Code); \
  resp_body = stock_replies::Code; \
}

server_handler::server_handler(mmstore &mms, std::string const& document_root)
: handler(handler::read), mms_(mms), document_root_(document_root)
{}

server_handler::~server_handler()
{
  std::cerr << "server handler freed\n";
}

void server_handler::on_request(
    boost::system::error_code const &err,
    http::entity::request const &request,
    http::connection_ptr conn)
{
  using namespace std;

  handler::on_request(err, request, conn);

  string request_path(request.query.path);
  string extension, full_path;
  char const *resp_body(0);

  // Request path must be absolute and not contain "..".
  if (request_path.empty() || request_path[0] != '/'
      || request_path.find("..") != string::npos)
  {
    CREATE_RESPONSE(bad_request);
  }else{
    // If path ends in slash (i.e. is a directory) then add "index.html".
    if (request_path[request_path.size() - 1] == '/')
      request_path += "index.html";

    // Determine the file extension.
    size_t last_slash_pos = request_path.find_last_of("/");
    size_t last_dot_pos = request_path.find_last_of(".");

    if (last_dot_pos != string::npos && last_dot_pos > last_slash_pos)
      extension = request_path.substr(last_dot_pos + 1);

    full_path = document_root_ + request_path;

    if(mms_.is_in(full_path)){
      CREATE_RESPONSE(ok);
    }else{
      CREATE_RESPONSE(not_found);
    }
  }

  if(resp_.status_code == (unsigned int)status_type::ok){
    resp_.headers.resize(2);
    resp_.headers[0].name = "Content-Type";
    resp_.headers[0].value = mime_types::extension_to_type(extension);
    resp_.headers[1].name = "Content-Length";
    resp_.headers[1].value = 
      boost::lexical_cast<string>(mms_.get_max_size(full_path));
    // TODO centerialize bandwidth control
    mms_handler_.reset(
      new mmstore_handler(
        mms_, full_path, handler::read, 4, 8192));
  }else{
    resp_.headers.resize(2);
    resp_.headers[0].name = "Content-Type";
    resp_.headers[0].value = "text/html";
    resp_.headers[1].name = "Content-Length";
    resp_.headers[1].value = 
      boost::lexical_cast<string>(strlen(resp_body));
  }

  // Fill buffer
  connection()->io_buffer().consume(connection()->io_buffer().size());
  {
    ostream os(&connection()->io_buffer());
    os << resp_ << resp_body;
  }
  
  send_header();
  return;
}

void server_handler::send_header()
{
  boost::asio::async_write(
    connection()->socket(),
    connection()->io_buffer(),
    boost::asio::transfer_at_least(1),
    boost::bind(&server_handler::handle_send_header, this, _1,_2));
}

void server_handler::handle_send_header(
  boost::system::error_code const &err,
  boost::uint32_t length)
{
  if(!err){
    if(connection()->io_buffer().size()){
      send_header();
    }else{
      mms_handler_->on_request(err, request(), connection());
    }
  }else{
    std::cerr  << __FILE__ <<  ":" << __LINE__ << ": " << err.message() << "\n";
  }
}


} // namespace http

#include "hanlder.hpp"

handler::handler(mode_t mode)
: mode_(mode), request_(0), response_(0)
{}

handler::~handler()
{}

void handler::on_response(
    boost::system::error_code const &err,
    http::entity::response const &response,
    http::connection_ptr conn)
{}

void handler::on_request(
    boost::system::error_code const &err,
    http::entity::request const &request,
    http::connection_ptr conn)
{}

void handler::notify(boost::system::error_code const &err)
{}

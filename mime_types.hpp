#ifndef GAISWT_MIME_TYPES_HPP
#define GAISWT_MIME_TYPES_HPP

#include <string>

namespace http {
namespace mime_types {

/// Convert a file extension into a MIME type.
std::string extension_to_type(const std::string& extension);

} // namespace mime_types
} // namespace http

#endif // GAISWT_MIME_TYPES_HPP

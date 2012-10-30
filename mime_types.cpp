#include "mime_types.hpp"

namespace http {
namespace mime_types {

struct mapping
{
  const char* extension;
  const char* mime_type;
} mappings[] =
{
  { "", "text/plain" },
  { "flv", "video/flv"},
  { "gif", "image/gif" },
  { "htm", "text/html" },
  { "html", "text/html" },
  { "js", "text/javascript" },
  { "jpg", "image/jpeg" },
  { "mp4", "video/mp4" },
  { "png", "image/png" },
  { "swf", "application/x-shockwave-flash" },
  { 0, 0 } // Marks end of list.
};

std::string extension_to_type(const std::string& extension)
{
  for (mapping* m = mappings; m->extension; ++m)
  {
    if (m->extension == extension)
    {
      return m->mime_type;
    }
  }

  return "text/plain";
}

} // namespace mime_types
} // namespace http

#ifndef RAVELERROR_H
#include <stdexcept>
#include <string>

namespace ravel
{
  struct RavelError: public std::runtime_error
  {
    RavelError(const std::string& err): std::runtime_error(err) {}
  };
}
#endif

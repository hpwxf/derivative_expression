#include "Demangle.hpp"

#include <cxxabi.h>
#include <memory>

std::string
demangle(const std::string_view mangled)
{
  int status = -1;

  char* cxa_demangled = abi::__cxa_demangle(mangled.data(), NULL, NULL, &status);
  if (status == 0) {
    std::string demangled{cxa_demangled};
    free(cxa_demangled);
    return demangled;
  } else {
    return std::string{mangled};
  }
}

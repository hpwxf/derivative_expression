#ifndef DEMANGLE_HPP
#define DEMANGLE_HPP

#include <string>
#include <typeinfo>

std::string demangle(const std::string_view mangled);

template <typename T>
inline std::string demangle() {
  return demangle(typeid(T).name());
}

#endif  // DEMANGLE_HPP

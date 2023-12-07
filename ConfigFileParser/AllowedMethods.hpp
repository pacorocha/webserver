#ifndef ALLOWEDMETHODS_HPP
#define ALLOWEDMETHODS_HPP

#include "ConfigFileParserExcept.hpp"
#include <vector>
#include <istream>

namespace internals {

class AllowedMethods {
public:
  AllowedMethods();
  ~AllowedMethods();
  AllowedMethods(std::istream &file) throw(BadFormat);

  bool is_method_allowed(std::string const &method) const;

private:
  std::vector<std::string> _methods;

  bool _is_a_valid_method(std::string const &method) const;
}; // AllowedMethods

} // internals

#endif // !ALLOWEDMETHODS_HPP

#include "AllowedMethods.hpp"
#include "ConfigFileParserExcept.hpp"
#include "ServersParser.hpp"
#include <string>
#include <vector>

using namespace internals;

AllowedMethods::AllowedMethods() {}
AllowedMethods::~AllowedMethods() {}

AllowedMethods::AllowedMethods(std::istream &file) throw(BadFormat) {
  std::string value;

  while (true) {
    file >> value;

    if (*--value.end() == ';') {

      ServersParser::remove_semicolon(value);

      if (!_is_a_valid_method(value))
        throw BadFormat();

      _methods.push_back(value);
      break;

    } else if (!_is_a_valid_method(value) || file.eof()) {
      throw BadFormat();
    }

    _methods.push_back(value);
  }
}

bool AllowedMethods::is_method_allowed(std::string const &method) const {
  bool found(false);
  std::vector<std::string>::const_iterator it(_methods.begin());

  if (_methods.empty())
    return false;

  for (; it != _methods.end(); ++it) {
    if (*it == method) {
      found = true;
      break;
    }
  }

  return found;
}

bool AllowedMethods::_is_a_valid_method(std::string const &method) const {
  return method == "GET" || method == "POST" || method == "DELETE";
}

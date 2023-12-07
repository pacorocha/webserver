#include "ConfigFileParserExcept.hpp"

using namespace internals;

BadFormat::BadFormat() {}

char const *BadFormat::what() const throw() {
  return "Config File doesn't have a valid syntax";
}

InvalidModule::InvalidModule() {}

char const *InvalidModule::what() const throw() {
  return "Invalid server modules in config file";
}

LocationNotFound::LocationNotFound() {}

char const *LocationNotFound::what() const throw() {
  return "Location with this uri no found !";
}

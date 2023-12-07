#include "ClientMaxBodySize.hpp"
#include "ConfigFileParserExcept.hpp"
#include "ServersParser.hpp"
#include <ios>
#include <iostream>

using namespace internals;

ClientMaxBodySize::ClientMaxBodySize() : _size(1), _unity('m') {}
ClientMaxBodySize::~ClientMaxBodySize() {}

ClientMaxBodySize::ClientMaxBodySize(std::istream &file) throw(BadFormat)
  : _size(1), _unity('m') {

  std::string value;
  std::string::const_iterator letter_position;

  value = ServersParser::extract_without_change_position(file);

  if (*--value.end() != ';')
    throw BadFormat();

  letter_position = value.end() - 2;

  for (std::string::const_iterator it = value.begin(); it != value.end(); ++it)
  {
    if ((it + 1) == value.end() && *it == ';')
      break;

    if (isalpha(*it)) {

      if (it != letter_position || !_is_valid_unity(*it))
        throw BadFormat();

    } else if (!isdigit(*it))
      throw BadFormat();
  }

  file >> _size;

  if (!isdigit(*letter_position))
    file >> _unity;

  ServersParser::remove_semicolon(file);
}

ClientMaxBodySize &
ClientMaxBodySize::operator=(ClientMaxBodySize const &other) {
  _size = other._size;
  _unity = other._unity;
  return *this;
}

int ClientMaxBodySize::get_size() const { return _size; }
char ClientMaxBodySize::get_unity() const { return _unity; }

bool ClientMaxBodySize::_is_valid_unity(char unity) const {
  return unity == 'k' || unity == 'g' || unity == 'm' || unity == 'K' ||
         unity == 'G' || unity == 'M';
}

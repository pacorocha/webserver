#include "Index.hpp"
#include "ConfigFileParserExcept.hpp"
#include "ServersParser.hpp"
#include <cctype>
#include <ios>
#include <string>

using namespace internals;

Index::Index() {}
Index::~Index() {}

Index::Index(std::istream &file) throw(BadFormat) {
  std::string value;

  while (true) {
    file >> value;

    for (std::string::iterator it = value.begin(); it != value.end(); ++it) {
      if ((it + 1) == value.end() && *it == ';')
        break;

      if (!_is_valid_character_to_unix_file_name(*it))
        throw BadFormat();
    }

    if (*--value.end() == ';') {

      ServersParser::remove_semicolon(value);
      _files.push_back(value);
      break;

    } else if (file.eof())
      throw BadFormat();

    _files.push_back(value);
  }
}

bool Index::_is_valid_character_to_unix_file_name(char c) const {
  return isalnum(c) || c == '.' || c == '_';
}

Index::iterator Index::begin() const { return _files.begin(); }
Index::iterator Index::end() const { return _files.end(); }

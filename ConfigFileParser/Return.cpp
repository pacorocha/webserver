#include "Return.hpp"
#include "ConfigFileParserExcept.hpp"
#include "ServersParser.hpp"
#include <ios>
#include <istream>
#include <string>

using namespace internals;

Return::Return() {}
Return::~Return() {}

Return::Return(std::istream &file) throw(BadFormat) {
  std::string value;

  file >> ServersParser::parser_number;

  if (file.fail())
    throw BadFormat();

  file >> _code;

  value = ServersParser::extract_without_change_position(file);

  if (value == ";")
    ServersParser::remove_semicolon(file);
  else {

    if (*--value.end() != ';')
      throw BadFormat();

    file >> _text_or_url;

    ServersParser::remove_semicolon(_text_or_url);
  }
}

int Return::get_code() const { return _code; }

std::string const &Return::get_content() const { return _text_or_url; }

bool Return::is_redirect() const {
  return _code == 301 || _code == 302 || _code == 303 || _code == 307;
}

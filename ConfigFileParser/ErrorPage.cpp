#include "ErrorPage.hpp"
#include "ConfigFileParserExcept.hpp"
#include "ServersParser.hpp"
#include <cctype>
#include <ios>
#include <iostream>
#include <istream>
#include <string>
#include <vector>

using namespace internals;

ErrorPage::ErrorPage() {}
ErrorPage::~ErrorPage() {}

ErrorPage::ErrorPage(std::istream &file) {
  std::string value;
  int code;

  while (true) {
    file >> ServersParser::parser_number;

    if (file.fail()) {

      file.clear();
      file >> _page;

      if (!_codes.empty() && *_page.begin() == '/' && *--_page.end() == ';')
        break;

      throw BadFormat();
    }

    file >> code;

    if (code < 300 || code > 599)
      throw BadFormat();

    _codes.push_back(code);
  }

  ServersParser::remove_semicolon(_page);
}

std::string const &ErrorPage::get_page() const { return _page; }

bool ErrorPage::search(int code) const {
  bool founded = false;

  for (std::vector<int>::const_iterator it = _codes.begin(); it != _codes.end();
       ++it) {

    if (*it == code) {
      founded = true;
      break;
    }
  }

  return founded;
}

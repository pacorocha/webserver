#include "ConfigFileParser.hpp"
#include "ConfigFileParserExcept.hpp"
#include "ServersParser.hpp"
#include <iostream>
#include <istream>
#include <string>

using namespace web;

ConfigFileParser::ConfigFileParser() {}
ConfigFileParser::~ConfigFileParser() {}

int ConfigFileParser::parser(char const file_path[]) throw() {
  std::ifstream file;
  std::string current_key;
  int status = 0;

  file.open(file_path);

  if (file.fail())
    return 1;

  while (file >> current_key, !file.eof()) {

    if (current_key == "server") {

      try {

        _servers.push_back(internals::ServersParser(file));

      } catch (internals::BadFormat const &error) {

        std::cerr << "[ERROR]: " << error.what() << std::endl;
        status = 1;
        break;
      }
    } else {
      std::cerr << "[ERROR]: " << internals::BadFormat().what() << std::endl;
      status = 1;
      break;
    }

    current_key.clear();
  }

  file.close();

  return status;
}

ConfigFileParser::iterator ConfigFileParser::begin() const {
  return _servers.begin();
}

ConfigFileParser::iterator ConfigFileParser::end() const {
  return _servers.end();
}

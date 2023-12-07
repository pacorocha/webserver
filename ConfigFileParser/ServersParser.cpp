#include "ServersParser.hpp"
#include "ConfigFileParserExcept.hpp"
#include <stdexcept>

using namespace internals;

ServersParser::ServersParser() {}
ServersParser::~ServersParser() {}

ServersParser::ServersParser(std::istream &file) throw(BadFormat)
    : _client_max_body_size(), _listen_already_set(false),
      _client_size_already_set(false) {

  std::string current_value;
  int angular_brackets_count = 0;

  file >> current_value;

  if (current_value != "{")
    throw BadFormat();

  angular_brackets_count = 1;

  while (angular_brackets_count == 1) {

    if (file.eof())
      throw BadFormat();

    file >> current_value;

    if (current_value == "listen") {

      if (_listen_already_set)
        throw BadFormat();

      file >> parser_number;

      validate_file_status(file);

      file >> _listen;
      remove_semicolon(file);

      _listen_already_set = true;

    } else if (current_value == "server_name") {

      if (!_server_name.empty())
        throw BadFormat();

      file >> parser_server_name;

      validate_file_status(file);

      file >> _server_name;
      remove_semicolon(_server_name);

    } else if (current_value == "error_page") {

      ErrorPage error_page(file);

      _error_pages.push_back(error_page);

    } else if (current_value == "root") {

      if (!_root.empty())
        throw BadFormat();

      file >> parser_root;

      validate_file_status(file);

      file >> _root;
      remove_semicolon(_root);

    } else if (current_value == "client_max_body_size") {

      if (_client_size_already_set)
        throw BadFormat();

      _client_max_body_size = ClientMaxBodySize(file);
      _client_size_already_set = true;

    } else if (current_value == "location") {
      std::string path(extract_without_change_position(file));

      if (*path.begin() != '/' || path.find(';') != std::string::npos)
        throw BadFormat();

      _locations[path] = Location(file);

    } else if (current_value == "}") {
      angular_brackets_count = 0;

    } else if (current_value == "cgi") {
      std::string extension, binary;

      file >> extension;

      if (*extension.begin() != '.')
        throw BadFormat();

      file >> binary;

      if (*--binary.end() != ';')
        throw BadFormat();

      ServersParser::remove_semicolon(binary);

      _cgi.first = extension;
      _cgi.second = binary;

    } else
      throw BadFormat();
  }
}

std::istream &ServersParser::parser_autoindex(std::istream &stream) {
  std::string value(extract_without_change_position(stream));

  if (value != "on;" && value != "off;")
    stream.setstate(std::ios_base::failbit);

  return stream;
}

std::istream &ServersParser::parser_root(std::istream &stream) {
  std::string value(extract_without_change_position(stream));

  if (!(*value.begin() == '/' && *--value.end() == ';'))
    stream.setstate(std::ios_base::failbit);

  return stream;
}

std::istream &ServersParser::parser_server_name(std::istream &stream) {
  std::string value(extract_without_change_position(stream));
  std::string::iterator it;

  it = value.begin();

  while (it != value.end()) {

    if ((it + 1) == value.end() && *it == ';') {
      ++it;
      break;
    }

    if (!isalpha(*it) && *it != '.')
      break;

    ++it;
  }

  if (it != value.end())
    stream.setstate(std::ios_base::failbit);

  return stream;
}

std::istream &ServersParser::parser_number(std::istream &stream) {
  std::string value(extract_without_change_position(stream));
  std::string::iterator it;

  it = value.begin();

  while (it != value.end()) {

    if ((it + 1) == value.end() && *it == ';') {
      ++it;
      break;
    }

    if (!isdigit(*it))
      break;
    ++it;
  }

  if (it != value.end())
    stream.setstate(std::ios_base::failbit);

  return stream;
}

void ServersParser::remove_semicolon(std::istream &stream) { stream.get(); }
bool ServersParser::has_semicolon_at_the_end(std::istream &file) {
  std::string value(extract_without_change_position(file));

  return *--value.end() == ';';
}

void ServersParser::remove_semicolon(std::string &value) {
  value.erase(--value.end());
}

int ServersParser::get_listen() const { return _listen; }
std::string const &ServersParser::get_root() const { return _root; }

std::string const &ServersParser::get_server_name() const {
  return _server_name;
}

std::string ServersParser::get_error_page(int code) const {
  std::vector<ErrorPage>::const_iterator it = _error_pages.begin();
  std::string page;

  for (; it != _error_pages.end(); ++it) {

    if (it->search(code))
      page = it->get_page();
  }

  return page;
}

bool ServersParser::has_error_pages() const { return !_error_pages.empty(); }

void ServersParser::validate_file_status(std::istream &file) throw(BadFormat) {

  if (file.fail() || !has_semicolon_at_the_end(file))
    throw BadFormat();
}

unsigned int ServersParser::get_max_body_size() const {
  int size = _client_max_body_size.get_size();
  char unity = tolower(_client_max_body_size.get_unity());
  unsigned int size_in_bytes;

  if (unity == 'k')

    size_in_bytes = size * 1e+3;

  else if (unity == 'm')

    size_in_bytes = size * 1e+6;

  else
    size_in_bytes = size * 1e+9;

  return size_in_bytes;
}

std::string ServersParser::extract_without_change_position(std::istream &file) {
  std::string value;
  std::streampos pos;

  pos = file.tellg();
  file >> value;
  file.seekg(pos);

  return value;
}

ServersParser::location_type
ServersParser::get_location(std::string const &uri) const
    throw(LocationNotFound) {
  try {

    return _locations.at(uri);

  } catch (std::out_of_range const &e) {
    throw LocationNotFound();
  }
}

std::string const& ServersParser::get_cgi_binary() const { return _cgi.second; }
std::string const& ServersParser::get_cgi_extension() const {
  return _cgi.first;
}

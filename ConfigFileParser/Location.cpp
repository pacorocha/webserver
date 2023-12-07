#include "Location.hpp"
#include "AllowedMethods.hpp"
#include "ConfigFileParserExcept.hpp"
#include "ServersParser.hpp"
#include <stdexcept>

using namespace internals;

Location::Location() {}
Location::~Location() {}

Location::Location(std::istream &file) throw(BadFormat)
    : _allowed_methods_already_set(false), _return_already_set(false) {

  std::string current_value;
  int angular_brackets_count;

  current_value = ServersParser::extract_without_change_position(file);

  _validate_path(current_value);

  file >> _path;

  file >> current_value;

  if (current_value != "{")
    throw BadFormat();

  angular_brackets_count = 1;

  while (angular_brackets_count == 1) {

    if (file.eof())
      throw BadFormat();

    file >> current_value;

    if (current_value == "root") {
      file >> ServersParser::parser_root;

      ServersParser::validate_file_status(file);

      file >> _root;
      ServersParser::remove_semicolon(_root);

    } else if (current_value == "index") {

      _indices.push_back(Index(file));

    } else if (current_value == "return") {

      if (has_return())
        throw BadFormat();

      _return = Return(file);
      _return_already_set = true;

    } else if (current_value == "autoindex") {
      std::string value;

      file >> ServersParser::parser_autoindex;

      ServersParser::validate_file_status(file);

      file >> value;
      _autoindex = value == "on;" ? true : false;

    } else if (current_value == "}") {

      angular_brackets_count = 0;

    } else if (current_value == "location") {
      std::string path(ServersParser::extract_without_change_position(file));

      if (*path.begin() != '/' || path.find(';') != std::string::npos)
        throw BadFormat();

      _locations[path] = Location(file);

    } else if (current_value == "allowed_methods") {

      if (_allowed_methods_already_set)
        throw BadFormat();

      _allowed_methods = AllowedMethods(file);
      _allowed_methods_already_set = true;

    } else
      throw BadFormat();
  }
}

void Location::_validate_path(std::string const &path_to_validate) const
    throw(BadFormat) {

  if (*path_to_validate.begin() != '/' ||
      path_to_validate.find(';') != std::string::npos)
    throw BadFormat();
}

std::string const &Location::get_uri() const { return _path; }

bool Location::has_return() const { return _return_already_set; }

std::string const &Location::get_root() const { return _root; }

Location::indices_iterator Location::begin_indices() const {
  return _indices.begin();
}

Location::indices_iterator Location::end_indices() const {
  return _indices.end();
}

Location::return_type Location::get_return() const { return _return; }

bool Location::is_autoindex_activated() const { return _autoindex; }

Location const &Location::get_location(std::string const &uri) const {
  try {

    Location const &searched = _locations.at(uri);
    return searched;

  } catch (std::out_of_range const &e) {
    throw LocationNotFound();
  }
}

bool Location::is_method_allowed(std::string const &method) const {
  return _allowed_methods.is_method_allowed(method);
}

bool Location::has_indices() const { return !_indices.empty(); }

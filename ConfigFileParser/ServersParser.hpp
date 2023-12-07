#ifndef SERVERSPARSER_HPP
#define SERVERSPARSER_HPP

#include "ClientMaxBodySize.hpp"
#include "ConfigFileParserExcept.hpp"
#include "ErrorPage.hpp"
#include "Location.hpp"
#include <istream>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace internals {

class ServersParser {
public:
  ServersParser();
  ServersParser(std::istream &file) throw(BadFormat);
  ~ServersParser();

  typedef Location const &location_type;

  static std::istream &parser_number(std::istream &stream);
  static std::istream &parser_server_name(std::istream &stream);
  static std::istream &parser_root(std::istream &stream);
  static std::istream &parser_autoindex(std::istream &stream);
  static bool has_semicolon_at_the_end(std::istream &file);
  static void remove_semicolon(std::istream &stream);
  static void remove_semicolon(std::string &value);
  static void validate_file_status(std::istream &file) throw(BadFormat);
  static std::string extract_without_change_position(std::istream &file);

  // Getters
  int get_listen() const;
  std::string const &get_server_name() const;
  std::string const &get_root() const;

  // ErrorPage methods
  bool has_error_pages() const;
  std::string get_error_page(int code) const;

  // ClientMaxBodySize
  unsigned int get_max_body_size() const;

  // Locations
  location_type get_location(std::string const &uri) const
      throw(LocationNotFound);

  //Cgi
  std::string const &get_cgi_extension() const;
  std::string const &get_cgi_binary() const;

private:
  int _listen;
  std::string _server_name;
  std::string _root;
  std::vector<ErrorPage> _error_pages;
  ClientMaxBodySize _client_max_body_size;
  std::map<std::string, Location> _locations;
  std::pair<std::string, std::string> _cgi;

  bool _listen_already_set;
  bool _client_size_already_set;

}; // ServersParser

} // namespace internals

#endif // !SERVERSPARSER_HPP

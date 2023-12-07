#include "Utils.hpp"
#include "HttpServer.hpp"
#include <sstream>
#include <string>
#include <vector>

using namespace internals;

std::string int_to_string(int value) {
  std::ostringstream stream;

  stream << value;

  return stream.str();
}

int string_to_int(std::string const &value) {
  std::stringstream stream;
  int value_as_int;

  stream << value;

  stream >> value_as_int;

  return value_as_int;
}

int init_servers_config(web::ConfigFileParser const &parser,
                        std::vector<web::HttpServer *> &servers_config) {
  web::ConfigFileParser::iterator it;

  for (it = parser.begin(); it != parser.end(); ++it) {
    try {

      servers_config.push_back(new web::HttpServer(it->get_listen(), *it));

    } catch (internals::SetupListenerError const &) {
      clear_http_servers(servers_config);
      return 1;
    }
  }
  return 0;
}

void clear_http_servers(std::vector<web::HttpServer *> &servers_config) {
  std::vector<web::HttpServer *>::reverse_iterator rit;

  for (rit = servers_config.rbegin(); rit != servers_config.rend(); ++rit)
    delete *rit;
}

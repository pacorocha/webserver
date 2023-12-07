#ifndef HTTPRESPONSEPARSER_HPP
#define HTTPRESPONSEPARSER_HPP

#include "HttpRequestParser.hpp"
#include "HttpResponseParserExcept.hpp"
#include "HttpServer.hpp"
#include "Index.hpp"
#include "Location.hpp"
#include "ServersParser.hpp"
#include <map>
#include <string>
#include <vector>

namespace internals {

enum StatusCode {
  OK = 200,
  NotFound = 404,
  MethodNotAllowed = 405,
  Forbidden = 403,
  MovedPermanently = 301,
  Found = 302,
  SeeOther = 303,
  TemporarilyRedirect = 307,
  BadRequest = 400,
  ServerError = 500,
  Created = 201,
  EntityTooLarge = 413
};

}

namespace web {

class HttpResponseParser {
public:
  HttpResponseParser();
  ~HttpResponseParser();

  static int send(int fd, std::string const &response);

  static std::string build(int fd, HttpServer const &http_server);

  static internals::Location const *
  match_precise(std::string const &uri, internals::ServersParser const &server);

  static std::string get_code_message(int code);

  static std::string get_path(std::string const &uri, std::string const &root);

  static std::string set_error_response(internals::ServersParser const &server,
                                        int code);

  static bool is_dir(std::string const &path);

  static void append_index(
      std::string &path,
      internals::Location const *location) throw(internals::NotFoundValidIndex);

  static void check_access(std::string const &path,
                           int permission) throw(internals::NotFoundExcept,
                                                 internals::NoPermissionToRead);

  static std::string const &get_mime_type(std::string const &file_extension);

  static std::string get_file_extension(std::string const &path);

  static std::string const &get_default_content();

  static char const end_of_response[];
  static char const end_of_line[];

private:
  static std::string const _get_valid_index(internals::Index const &indices,
                                            std::string const &path);

  static std::string _get_default_content();

  static std::map<std::string, std::string> _mime_type;

  static void _fill_mime_type(void);

  static std::string const _default_content;
};

} // namespace web

#endif // !HTTPRESPONSEPARSER_HPP

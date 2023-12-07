#ifndef GETRESPONSEPARSER_HPP
#define GETRESPONSEPARSER_HPP

#include "HttpRequestParser.hpp"
#include "HttpServer.hpp"
#include "Return.hpp"
#include "ServersParser.hpp"
#include <sstream>
#include <string>

namespace internals {

class GetResponseParser {
public:
  GetResponseParser();
  ~GetResponseParser();

  static std::string build(web::HttpRequestParser const &request,
                           web::HttpServer const &http_server);

  static std::string setup_response(std::string const &content,
                                    std::string const &file_extension,
                                    int code);

  static std::string get_error_page(internals::ServersParser const &server,
                                    int code);

private:
  static std::string _get_content(std::string const &path);
  static std::string _setup_redirection(std::string const &uri,
                                        std::string const &host, int code);

  static std::string _setup_return(internals::Return const &module);

  static std::string _format_list_of_links(std::string const &path,
                                           std::string const &uri);

  static bool _is_a_cgi_file(std::string const &path,
                             std::string const &cgi_extension);
};

} // namespace internals

#endif // !GETRESPONSEPARSER_HPP

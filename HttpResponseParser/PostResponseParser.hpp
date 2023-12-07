#ifndef POSTRESPONSEPARSER_HPP
#define POSTRESPONSEPARSER_HPP

#include "HttpRequestParser.hpp"
#include "HttpServer.hpp"
#include "ServersParser.hpp"
#include <map>
#include <string>
#include <vector>
namespace internals {

struct MultipartForm {
  MultipartForm(std::string const &file_name, std::string const &content_type,
                std::string const &content);

  std::string const file_name;
  std::string const content_type;
  std::string const content;
};

class PostResponse {
public:
  static std::string build(web::HttpRequestParser const &requests,
                           internals::ServersParser const &server);

private:
  static bool _is_multipartform(web::HttpRequestParser const &request);

  static MultipartForm
  _parser_multipartform(web::HttpRequestParser const &request);

  static MultipartForm
  _parser_multipartform_payload(std::string const &body,
                                std::string const &delimiter);
};

} // namespace internals

#endif // !POSTRESPONSEPARSER_HPP

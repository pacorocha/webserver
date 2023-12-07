#include "PostResponseParser.hpp"
#include "CGI.hpp"
#include "GetResponseParser.hpp"
#include "HttpRequestParser.hpp"
#include "HttpResponseParser.hpp"
#include "Location.hpp"
#include "ServersParser.hpp"
#include <cstddef>
#include <string>

using namespace internals;

#define BOUNDARY_LENGTH 9

MultipartForm::MultipartForm(std::string const &file_name,
                             std::string const &content_type,
                             std::string const &content)
    : file_name(file_name), content_type(content_type), content(content) {}

std::string PostResponse::build(web::HttpRequestParser const &request,
                                internals::ServersParser const &server) {
  Location const *location;
  std::string path;
  std::pair<int, std::string> result;
  CGI cgi;

  std::string const &uri = request.get("Request-URI");
  location = web::HttpResponseParser::match_precise(uri, server);

  if (location) {

    if (!location->is_method_allowed(request.get("Method"))) {
      return GetResponseParser::setup_response(
          GetResponseParser ::get_error_page(server, MethodNotAllowed), ".html",
          MethodNotAllowed);
    }

    path = location->get_root();

    MultipartForm const &content = _parser_multipartform(request);
    result = cgi.processRequest(request, content, path, server);
    if (result.first) {
      return internals::GetResponseParser::setup_response(
          internals::GetResponseParser::get_error_page(server, ServerError),
          ".html", ServerError);
    }

    return internals::GetResponseParser::setup_response("", "", Created);
  } else {
    return internals::GetResponseParser::setup_response(
        internals::GetResponseParser::get_error_page(server, Forbidden),
        ".html", Forbidden);
  }
}

bool PostResponse::_is_multipartform(web::HttpRequestParser const &request) {
  std::string const &content_type = request.get("content-type");

  return content_type.find("multipart/form-data") != std::string::npos;
}

MultipartForm
PostResponse::_parser_multipartform(web::HttpRequestParser const &request) {

  std::string delimiter(request.get("Content-Type"));
  delimiter = delimiter.substr(delimiter.find("boundary=") + BOUNDARY_LENGTH);
  delimiter.insert(0, "--");

  return _parser_multipartform_payload(request.get("Body"), delimiter);
}

MultipartForm
PostResponse::_parser_multipartform_payload(std::string const &body,
                                            std::string const &delimiter) {
  std::string file_name, content, content_type;
  std::string::size_type pos, length;

  pos = body.find("filename=\"") + 10;
  length = body.find('"', pos + 1);

  file_name = body.substr(pos, length - pos);

  pos = body.find("Content-Type:") + 14;

  for (size_t end_of_line = body.find("\r\n", pos); pos != end_of_line; ++pos)
    content_type.push_back(body[pos]);

  pos = body.find("\r\n\r\n") + 4;

  content = body.substr(pos, body.find_last_of(delimiter + "--") - pos -
                                 delimiter.length() - 4);

  return MultipartForm(file_name, content_type, content);
}

#include "GetResponseParser.hpp"
#include "CGI.hpp"
#include "HttpResponseParser.hpp"
#include "HttpResponseParserExcept.hpp"
#include "HttpServer.hpp"
#include "Location.hpp"
#include "PostResponseParser.hpp"
#include "Return.hpp"
#include "ServersParser.hpp"
#include <cstdio>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <typeinfo>
#include <unistd.h>
#include <utility>

using namespace internals;

GetResponseParser::GetResponseParser() {}
GetResponseParser::~GetResponseParser() {}

std::string GetResponseParser::build(web::HttpRequestParser const &request,
                                     web::HttpServer const &http_server) {

  internals::ServersParser const &server = http_server.get_server_config();
  std::string const &uri = request.get("Request-URI");
  std::ostringstream stream;
  internals::Location const *location;
  std::string path;

  location = web::HttpResponseParser::match_precise(uri, server);

  if (location) {

    if (!location->is_method_allowed(request.get("Method"))) {
      stream << GetResponseParser::setup_response(
          get_error_page(server, MethodNotAllowed), ".html", MethodNotAllowed);
      return stream.str();
    }

    path = web::HttpResponseParser::get_path(uri, location->get_root());

    if (web::HttpResponseParser::is_dir(path)) {

      if (location->has_return()) {
        stream << GetResponseParser::_setup_return(location->get_return());
        return stream.str();

      } else if (*--path.end() != '/') {
        stream << GetResponseParser::_setup_redirection(
            uri, request.get("Host"), MovedPermanently);
        return stream.str();
      }

      try {

        web::HttpResponseParser::append_index(path, location);
        web::HttpResponseParser::check_access(path, R_OK);

      } catch (CommonException const &e) {

        if (typeid(e) == typeid(NotFoundExcept)) {
          if (location->is_autoindex_activated()) {
            return setup_response(_format_list_of_links(path, uri), ".html",
                                  OK);
          }
        }

        stream << GetResponseParser::setup_response(
            get_error_page(server, e.code), ".html", e.code);
        return stream.str();
      }

    } else {

      try {
        web::HttpResponseParser::check_access(path, F_OK | R_OK);
      } catch (CommonException const &e) {
        stream << GetResponseParser::setup_response(
            get_error_page(server, e.code), ".html", e.code);
        return stream.str();
      }
    }

    if (_is_a_cgi_file(path, server.get_cgi_extension())) {
      CGI cgi;
      std::pair<int, std::string> result;
      MultipartForm const content_parsed(uri.substr(uri.find_last_of('/')), "",
                                         "");

      result = cgi.processRequest(request, content_parsed, location->get_root(),
                                  server);

      if (result.first) {
        return internals::GetResponseParser::setup_response(
            internals::GetResponseParser::get_error_page(server, ServerError),
            ".html", ServerError);
      }

      stream << GetResponseParser::setup_response(
          result.second, web::HttpResponseParser::get_file_extension(".txt"),
          OK);
    } else {
      stream << GetResponseParser::setup_response(
          GetResponseParser::_get_content(path),
          web::HttpResponseParser::get_file_extension(path), OK);
    }

  } else {
    stream << GetResponseParser::setup_response(
        get_error_page(server, Forbidden), ".html", Forbidden);
  }

  return stream.str();
}

std::string
GetResponseParser::get_error_page(internals::ServersParser const &server,
                                  int code) {
  std::string error_page = server.get_error_page(code);

  if (error_page.empty())
    return web::HttpResponseParser::get_default_content();

  return GetResponseParser::_get_content(error_page);
}

std::string GetResponseParser::_setup_return(internals::Return const &module) {
  std::ostringstream stream;

  stream << web::HttpResponseParser::get_code_message(module.get_code())
         << web::HttpResponseParser::end_of_line;

  if (module.is_redirect()) {
    std::string const &content = web::HttpResponseParser::get_default_content();

    stream << "Location: " << module.get_content()
           << web::HttpResponseParser::end_of_line
           << "Content-Length: " << content.length()
           << web::HttpResponseParser::end_of_line << "Content-Type: "
           << web::HttpResponseParser::get_mime_type(".html")
           << web::HttpResponseParser::end_of_response << content;
  } else {

    stream << "Content-Length: " << module.get_content().length()
           << web::HttpResponseParser::end_of_line
           << "Content-Type: " << web::HttpResponseParser::get_mime_type(".txt")
           << web::HttpResponseParser::end_of_response << module.get_content();
  }

  return stream.str();
}

std::string GetResponseParser::_setup_redirection(std::string const &uri,
                                                  std::string const &host,
                                                  int code) {

  std::ostringstream stream;
  std::string const &content = web::HttpResponseParser::get_default_content();

  stream << web::HttpResponseParser::get_code_message(code)
         << web::HttpResponseParser::end_of_line
         << "Content-Length: " << content.length()
         << web::HttpResponseParser::end_of_line
         << "Content-Type: " << web::HttpResponseParser::get_mime_type(".html")
         << web::HttpResponseParser::end_of_line << "Connection: keep-alive"
         << web::HttpResponseParser::end_of_line << "Location: http://" << host
         << uri + '/' << web::HttpResponseParser::end_of_response << content;

  return stream.str();
}

std::string GetResponseParser::setup_response(std::string const &content,
                                              std::string const &file_extension,
                                              int code) {

  std::ostringstream stream;

  stream << web::HttpResponseParser::get_code_message(code)
         << web::HttpResponseParser::end_of_line
         << "Content-Length: " << content.length();

  if (!file_extension.empty()) {
    stream << web::HttpResponseParser::end_of_line << "Content-Type: "
           << web::HttpResponseParser::get_mime_type(file_extension);
  }
  stream << web::HttpResponseParser::end_of_response << content;

  return stream.str();
}

std::string GetResponseParser::_get_content(std::string const &path) {
  std::ifstream file(path.c_str());
  std::ostringstream stream;

  stream << file.rdbuf();

  return stream.str();
}

std::string GetResponseParser::_format_list_of_links(std::string const &path,
                                                     std::string const &uri) {
  std::ostringstream stream;
  DIR *dir;
  struct dirent *entity;
  char const link_format[] = "<a href=\"%s\">%s</a>";
  char formated[1024];
  std::string entity_name;

  dir = opendir((path.substr(0, path.find_last_of('/')).c_str()));

  stream << "<!DOCTYPE html>"
         << "<html>"
         << "<head><title>Index of " << uri << "</title></head>"
         << "<body>"
         << "<h1>Index of " << uri << "</h1>"
         << "<hr><pre>";

  while ((entity = readdir(dir))) {
    entity_name = entity->d_name;

    if (entity_name == ".")
      continue;

    if (entity->d_type == DT_DIR)
      entity_name.push_back('/');

    snprintf(formated, 1024, link_format, entity_name.c_str(),
             entity_name.c_str());

    stream << formated << "\n";
  }

  stream << "</pre><hr></body></html>";

  return stream.str();
}

bool GetResponseParser::_is_a_cgi_file(std::string const &path,
                                       std::string const &cgi_extension) {

  if (cgi_extension.empty())
    return false;
  return path.substr((path.end() - 4) - path.begin()).find(cgi_extension) !=
         std::string::npos;
}

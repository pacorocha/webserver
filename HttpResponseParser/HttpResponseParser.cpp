#include "HttpResponseParser.hpp"
#include "ConfigFileParserExcept.hpp"
#include "DeleteResponseParser.hpp"
#include "GetResponseParser.hpp"
#include "HttpRequestParser.hpp"
#include "HttpResponseParserExcept.hpp"
#include "HttpServer.hpp"
#include "Index.hpp"
#include "Location.hpp"
#include "PostResponseParser.hpp"
#include "ServersParser.hpp"
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace web;

std::map<std::string, std::string> HttpResponseParser::_mime_type;

char const HttpResponseParser::end_of_response[] = "\r\n\r\n";
char const HttpResponseParser::end_of_line[] = "\r\n";
std::string const HttpResponseParser::_default_content =
    HttpResponseParser::_get_default_content();

HttpResponseParser::HttpResponseParser() {}
HttpResponseParser::~HttpResponseParser() {}

int HttpResponseParser::send(int fd, const std::string &response) {
  return ::send(fd, response.c_str(), response.length(), 0);
}

std::string HttpResponseParser::build(int fd, HttpServer const &http_server) {

  HttpRequestParser const &request = http_server.get_request(fd);
  std::string method(request.get("Method"));
  std::ostringstream response;

  response << "HTTP/1.1 ";

  if (_mime_type.empty())
    _fill_mime_type();

  if (method == "GET") {
    response << internals::GetResponseParser::build(request, http_server);
  } else if (method == "POST") {
    response << internals::PostResponse::build(request,
                                               http_server.get_server_config());
  } else if (method == "DELETE") {
    response << internals::DeleteResponse::build(
        request, http_server.get_server_config());
  } else {
    response << "405 Method Not Allowed\r\n\r\n";
  }

  return response.str();
}

internals::Location const *
HttpResponseParser::match_precise(std::string const &uri,
                                  internals::ServersParser const &server) {

  std::string dir_location = uri.substr(0, uri.find_last_of('/'));

  try {
    return &server.get_location(uri);

  } catch (internals::LocationNotFound const &) {
  }

  try {
    return &server.get_location(uri + '/');
  } catch (internals::LocationNotFound const &) {
  }

  try {
    return &server.get_location(dir_location);
  } catch (internals::LocationNotFound const &) {
  }

  try {
    return &server.get_location(dir_location + '/');
  } catch (internals::LocationNotFound const &) {
  }

  return NULL;
}

std::string HttpResponseParser::get_code_message(int code) {
  switch (code) {
  case internals::OK:
    return "200 OK";
  case internals::BadRequest:
    return "400 Bad Request";
  case internals::NotFound:
    return "404 Not Found";
  case internals::MethodNotAllowed:
    return "405 Method Not Allowed";
  case internals::Forbidden:
    return "403 Forbidden";
  case internals::MovedPermanently:
    return "301 Moved Permanently";
  case internals::Found:
    return "302 Found";
  case internals::SeeOther:
    return "303 See Other";
  case internals::TemporarilyRedirect:
    return "307 Temporarily Redirect";
  case internals::EntityTooLarge:
    return "413 Payload Too Large";
  case internals::Created:
    return "201 Created";
  case internals::ServerError:
    return "500 Internal Server Error";
  default:
    return "";
  }
}

std::string HttpResponseParser::get_path(std::string const &uri,
                                         std::string const &root) {
  char *const cwd = getcwd(NULL, 0);
  std::string cwd_as_string(cwd);
  free(cwd);
  return root.empty() ? cwd_as_string + "/html" + uri : root + uri;
}

bool HttpResponseParser::is_dir(const std::string &path) {
  struct stat st;

  if (!stat(path.c_str(), &st))
    return S_ISDIR(st.st_mode) ? true : false;

  return false;
}

void HttpResponseParser::append_index(
    std::string &path,
    const internals::Location *location) throw(internals::NotFoundValidIndex) {

  internals::Location::indices_iterator it;
  std::string valid_index;

  if (location->has_indices()) {
    for (it = location->begin_indices(); it != location->end_indices(); ++it) {
      valid_index = _get_valid_index(*it, path);
    }

    if (valid_index.empty())
      throw internals::NotFoundValidIndex();

    path += valid_index;
  } else
    path += "index.html";
}

std::string const
HttpResponseParser::_get_valid_index(internals::Index const &indices,
                                     std::string const &path) {

  internals::Index::iterator it;

  for (it = indices.begin(); it != indices.end(); ++it) {
    if (!access((path + *it).c_str(), F_OK))
      return *it;
  }

  return "";
}

void HttpResponseParser::check_access(
    const std::string &path,
    int permission) throw(internals::NotFoundExcept,
                          internals::NoPermissionToRead) {

  if (!access(path.c_str(), permission))
    return;

  if (errno == ENOENT)
    throw internals::NotFoundExcept();

  if (errno == EINVAL) {
    if (permission & R_OK)
      throw internals::NoPermissionToRead();
  }
}

void HttpResponseParser::_fill_mime_type(void) {
  _mime_type[".html"] = "text/html";
  _mime_type[".htm"] = "text/html";
  _mime_type[".txt"] = "text/plain";
  _mime_type[".ico"] = "image/x-icon";
  _mime_type[".gif"] = "image/gif";
  _mime_type[".csv"] = "text/csv";
  _mime_type[".css"] = "text/css";
  _mime_type[".pdf"] = "application/pdf";
  _mime_type[".jpg"] = "image/jpeg";
  _mime_type[".png"] = "image/png";
  _mime_type[".jpeg"] = "image/jpg";
  _mime_type[".php"] = "application/x-httpd-php";
}

std::string const &
HttpResponseParser::get_mime_type(const std::string &file_extension) {
  try {
    std::string const &mime = _mime_type.at(file_extension);
    return mime;
  } catch (std::out_of_range const &) {
    return _mime_type[".txt"];
  }
}

std::string HttpResponseParser::get_file_extension(const std::string &path) {
  return path.substr(path.find_last_of('.'));
}

std::string HttpResponseParser::_get_default_content() {
  char *const cwd = getcwd(NULL, 0);
  std::string const cwd_as_string(cwd);
  free(cwd);
  std::ifstream file(cwd_as_string + "/html/error_page.html");
  std::ostringstream content;

  content << file.rdbuf();

  file.close();

  return content.str();
}

std::string const &HttpResponseParser::get_default_content() {
  return HttpResponseParser::_default_content;
}

std::string
HttpResponseParser::set_error_response(internals::ServersParser const &server,
                                       int code) {
  return "HTTP/1.1 " +
         internals::GetResponseParser::setup_response(
             internals::GetResponseParser::get_error_page(server, code),
             ".html", code);
}

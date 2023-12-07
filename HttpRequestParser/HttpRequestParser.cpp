#include "HttpRequestParser.hpp"
#include "HttpRequestParserExcpt.hpp"
#include "ServersParser.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <ios>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <strings.h>
#include <unistd.h>
#include <vector>

using namespace web;

bool HttpRequestParser::insensitiveCompare::compare::operator()(
    char const &c1, char const &c2) const {
  return tolower(c1) < tolower(c2);
}

bool HttpRequestParser::insensitiveCompare::operator()(
    std::string const &lhs, std::string const &rhs) const {
  return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                      rhs.end(), compare());
}

HttpRequestParser::HttpRequestParser()
    : _current(0), _http_index(0), _body_length(0), _chunk_size(0),
      _is_method_parsed(false), _is_uri_parsed(false),
      _is_http_version_parser(false), _request_line_parsed(false),
      _is_completed(false), _headers_parsed(false) {}

HttpRequestParser::~HttpRequestParser() {}

void HttpRequestParser::parser(
    char const buffer[], unsigned int length,
    internals::ServersParser const &
        server) throw(internals::BadRequestExcpt, internals::IncompletedRequest,
                      internals::EntityTooLargeExcpt) {

  std::string buffer_as_string(buffer, length);

  _parser(std::vector<char>(buffer_as_string.begin(), buffer_as_string.end()));

  if (get("Method") == "POST" && _is_completed) {
    if (get("body").length() > server.get_max_body_size())
      throw internals::EntityTooLargeExcpt();
  }
}

void HttpRequestParser::_parser(std::vector<char> const &request) throw(
    internals::BadRequestExcpt, internals::IncompletedRequest) {

  _set_request_line(request);
  _set_headers(request);

  _has_host_header();

  if (get("Method") == "POST") {
    try {

      if (get("Transfer-Encoding") == "chunked")
        _set_chunked_body(request);

    } catch (internals::KeyError const &) {
      _set_body(request);
    }
  }

  _is_completed = true;
}

unsigned int
HttpRequestParser::_get_chunk_size(std::vector<char> const &request) {
  std::string size_as_hex;
  std::stringstream stream;
  unsigned int result = 0;

  while (!_is_eol(request))
    size_as_hex.push_back(request[_current++]);

  _current += 2;

  stream << size_as_hex;
  stream >> std::hex >> result;

  return result;
}

void HttpRequestParser::_set_chunked_body(
    std::vector<char> const &request) throw(internals::IncompletedRequest) {
  static int chunk_size_left;
  int content_size = request.size() - _current, length;

  while (content_size > 0) {
    if (chunk_size_left == 0) {
      chunk_size_left = _get_chunk_size(request);
      content_size = request.size() - _current;

      if (!chunk_size_left) {
        _request["content-length"] = _chunk_size;
        return;
      }

      _chunk_size += chunk_size_left;
    }

    length = content_size > chunk_size_left ? chunk_size_left : content_size;

    _request["body"].append(request.data() + _current, length);

    _current += length;
    chunk_size_left -= length;

    _is_incomplete(request.size());

    content_size -= length;

    if (_is_eol(request))
      _current += 2;
  }
}

void HttpRequestParser::_set_body(std::vector<char> const &request) {
  unsigned int length = string_to_int(get("Content-Length"));
  int content_size = request.size() - _current;

  if (!_request["Body"].capacity())
    _request["Body"].reserve(length);

  while (_body_length < length) {
    _is_incomplete(request.size());
    _request["Body"].append(request.data() + _current, content_size);
    _body_length += content_size;
    _current += content_size;
  }
}

void HttpRequestParser::_set_headers(std::vector<char> const &request) {

  if (_headers_parsed)
    return;

  while (true) {
    _is_incomplete(request.size());

    if (_set_field_name(request)) {

      if (_is_eor(request)) {
        _current += 4;
        break;
      }
      _current += 2;
      continue;
    }

    _set_field_value(request);

    _request[_tmp_field_name].append(_tmp_field_value);
    _tmp_field_name.clear();
    _tmp_field_value.clear();

    if (_is_eor(request)) {
      _current += 4;
      break;
    }

    _current += 2;
  }

  _headers_parsed = true;
}

std::string const &HttpRequestParser::get(std::string const &key) const
    throw(internals::KeyError) {
  try {

    return _request.at(key);

  } catch (std::out_of_range const &e) {

    throw internals::KeyError();
  }
}

void HttpRequestParser::_set_request_line(
    std::vector<char> const &request) throw(internals::BadRequestExcpt,
                                            internals::IncompletedRequest) {

  if (_request_line_parsed)
    return;

  if (_parser_method(request) || _parser_uri(request) ||
      _parser_http_version(request) || _is_eor(request))
    throw internals::BadRequestExcpt();

  _request_line_parsed = true;
}

inline bool HttpRequestParser::_is_eol(std::vector<char> const &request) const {
  return request[_current] == '\r' && request[_current + 1] == '\n';
}

inline bool HttpRequestParser::_is_eor(std::vector<char> const &request) const {
  return (request[_current] == '\r' && request[_current + 1] == '\n' &&
          request[_current + 2] == '\r' && request[_current + 3] == '\n');
}

void HttpRequestParser::_set_field_value(std::vector<char> const &request) {

  if (!_request[_tmp_field_name].empty())
    _request[_tmp_field_name].push_back(',');

  while (isspace(request[_current]) && !_is_cr_or_nl(request[_current]))
    ++_current;

  while (!_is_eol(request)) {

    _is_incomplete(request.size());

    if (_is_cr_or_nl(request[_current]))
      throw internals::BadRequestExcpt();

    _tmp_field_value.push_back(request[_current]);
    ++_current;
  }
}

inline bool HttpRequestParser::_is_cr_or_nl(char c) const {
  return c == '\r' || c == '\n';
}

int HttpRequestParser::_set_field_name(std::vector<char> const &request) {

  if (!_tmp_field_name.empty())
    return 0;

  while (request[_current] != ':') {

    if (isspace(request[_current]))
      throw internals::BadRequestExcpt();

    _tmp_field_name.push_back(request[_current]);

    ++_current;

    _is_incomplete(request.size());

    if (_is_eol(request)) {
      _tmp_field_name.clear();
      return 1;
    }
  }

  ++_current;

  _is_incomplete(request.size());

  return 0;
}

int HttpRequestParser::_parser_http_version(std::vector<char> const &request) {
  char http_version[] = "HTTP/1.1";

  if (_is_http_version_parser)
    return 0;

  for (; _http_index < 8; ++_http_index, ++_current) {
    _is_incomplete(request.size());

    if (request[_current] != http_version[_http_index])
      return 1;
  }

  _is_incomplete(request.size());

  while (!_is_eol(request)) {

    if (request[_current] != ' ')
      return 1;

    ++_current;
    _is_incomplete(request.size());
  }

  if (_is_eor(request))
    throw internals::BadRequestExcpt();

  _current += 2;

  _is_http_version_parser = true;
  return 0;
}

int HttpRequestParser::_parser_uri(std::vector<char> const &request) {
  std::string::size_type pos;

  if (_is_uri_parsed)
    return 0;

  _is_incomplete(request.size());

  if (request[_current] != '/')
    return 1;

  do {
    _request["Request-URI"].push_back(request[_current]);
    ++_current;
    _is_incomplete(request.size());
  } while (request[_current] != ' ');

  _is_uri_parsed = true;

  while (request[_current] == ' ') {
    ++_current;
    _is_incomplete(request.size());
  }

  pos = _request["Request-URI"].find('?');

  if (pos != std::string::npos) {
    std::string &request_uri = _request["Request-URI"];
    _request["query-string"].insert(_request["query-string"].begin(),
                                    request_uri.begin() + pos + 1,
                                    request_uri.end());
    request_uri.erase(request_uri.begin() + pos, request_uri.end());
  }

  return 0;
}

int HttpRequestParser::_parser_method(std::vector<char> const &request) {
  char c;

  if (_is_method_parsed)
    return 0;

  do {
    c = request[_current];

    if (!isalpha(c) || (isalpha(c) && !isupper(c)))
      return 1;

    ++_current;

    _request["Method"].push_back(c);

    _is_incomplete(request.size());

  } while (request[_current] != ' ');

  _is_method_parsed = true;

  while (request[_current] == ' ') {
    ++_current;
    _is_incomplete(request.size());
  };

  return 0;
}

inline void HttpRequestParser::_is_incomplete(
    unsigned int request_length) throw(internals::IncompletedRequest) {
  if (!(_current < request_length)) {
    _current = 0;
    throw internals::IncompletedRequest();
  }
}

void HttpRequestParser::_has_host_header() const
    throw(internals::BadRequestExcpt) {

  try {
    get("host");
  } catch (internals::KeyError const &) {
    throw internals::BadRequestExcpt();
  }
}

bool HttpRequestParser::is_completed() const { return _is_completed; }

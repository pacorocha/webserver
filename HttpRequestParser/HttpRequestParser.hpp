#ifndef HTTPREQUESTPARSER_HPP
#define HTTPREQUESTPARSER_HPP

#include "HttpRequestParserExcpt.hpp"
#include "ServersParser.hpp"
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace web {

class HttpRequestParser {
  typedef std::map<std::string const, std::string> mapped_type;
  typedef mapped_type::value_type value_type;

public:
  HttpRequestParser();
  ~HttpRequestParser();

  void parser(char const buffer[], unsigned int length,
              internals::ServersParser const
                  &server) throw(internals::BadRequestExcpt,
                                 internals::IncompletedRequest,
                                 internals::EntityTooLargeExcpt);

  std::string const &get(std::string const &key) const
      throw(internals::KeyError);

  bool is_completed() const;

private:
  struct insensitiveCompare
      : std::binary_function<std::string, std::string, bool> {

    struct compare : std::binary_function<char, char, bool> {
      bool operator()(char const &c1, char const &c2) const;
    };

    bool operator()(std::string const &lhs, std::string const &rhs) const;
  };

  std::map<std::string const, std::string, insensitiveCompare> _request;
  std::string _tmp_field_name;
  std::string _tmp_field_value;
  unsigned int _current;
  unsigned int _http_index;
  unsigned int _body_length;
  unsigned int _chunk_size;
  bool _is_method_parsed;
  bool _is_uri_parsed;
  bool _is_http_version_parser;
  bool _request_line_parsed;
  bool _is_completed;
  bool _headers_parsed;

  void _parser(std::vector<char> const &request) throw(
      internals::BadRequestExcpt, internals::IncompletedRequest);

  void _set_chunked_body(std::vector<char> const &request) throw(
      internals::IncompletedRequest);
  unsigned int _get_chunk_size(std::vector<char> const &request);

  void _set_request_line(std::vector<char> const &request) throw(
      internals::BadRequestExcpt, internals::IncompletedRequest);

  void _set_headers(std::vector<char> const &request);
  void _set_body(std::vector<char> const &request);
  int _set_field_name(std::vector<char> const &request);
  void _set_field_value(std::vector<char> const &request);

  int _parser_method(std::vector<char> const &request);

  int _parser_uri(std::vector<char> const &request);
  int _parser_http_version(std::vector<char> const &request);
  inline bool _is_eol(std::vector<char> const &request) const;
  inline bool _is_eor(std::vector<char> const &request) const;

  inline void _is_incomplete(unsigned int request_length) throw(
      internals::IncompletedRequest);

  inline bool _is_cr_or_nl(char c) const;

  void _has_host_header() const throw(internals::BadRequestExcpt);
};

}; // namespace web

#endif // !HTTPREQUESTPARSER_HPP

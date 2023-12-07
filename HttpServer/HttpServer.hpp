#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include "ConfigFileParser.hpp"
#include "HttpRequestParser.hpp"
#include "HttpRequestParserExcpt.hpp"
#include "Server.hpp"
#include "ServersParser.hpp"
#include <map>
#include <vector>

namespace web {

class HttpServer : public Server {
public:
  HttpServer(int port, internals::ServersParser const &parser);
  ~HttpServer();

  // Requests Methods
  void add_request(int fd, char const buffer[],
                   unsigned int length) throw(internals::BadRequestExcpt,
                                              internals::IncompletedRequest,
                                              internals::EntityTooLargeExcpt);

  void clear_requests(int fd);

  Server::iterator close_connection(Server::iterator const &pos);
  internals::ServersParser const &get_server_config() const;
  HttpRequestParser const &get_request(int fd) const;

private:
  std::map<int, HttpRequestParser> _requests;
  internals::ServersParser const &server_config;

  typedef std::map<int, HttpRequestParser>::value_type value_type;

}; // HttpServer

} // namespace web

#endif // !HTTPSERVER_HPP

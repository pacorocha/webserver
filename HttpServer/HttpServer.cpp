#include "HttpServer.hpp"
#include "HttpRequestParser.hpp"
#include "HttpRequestParserExcpt.hpp"
#include "Server.hpp"
#include "ServersParser.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace web;

HttpServer::HttpServer(int port, internals::ServersParser const &parser)
    : Server(port), server_config(parser) {}

HttpServer::~HttpServer() {}

Server::iterator HttpServer::close_connection(Server::iterator const &pos) {
  clear_requests(pos->fd);
  return Server::close_connection(pos);
}

void HttpServer::add_request(
    int fd, char const buffer[],
    unsigned int length) throw(internals::BadRequestExcpt,
                               internals::IncompletedRequest,
                               internals::EntityTooLargeExcpt) {
  _requests[fd].parser(buffer, length, server_config);
}

void HttpServer::clear_requests(int fd) { _requests.erase(fd); }

internals::ServersParser const &HttpServer::get_server_config() const {
  return server_config;
}

HttpRequestParser const &HttpServer::get_request(int fd) const {
  return _requests.at(fd);
}

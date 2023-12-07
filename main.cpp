#include "ConfigFileParser.hpp"
#include "HttpRequestParserExcpt.hpp"
#include "HttpResponseParser.hpp"
#include "HttpServer.hpp"
#include "Server.hpp"
#include "Utils.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <string>
#include <strings.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

using namespace web;

#define BUFFER_SIZE 500

static void scan_sockets(HttpServer &server) {
  HttpServer::iterator it = server.begin();
  int readed, sent;
  char buffer[BUFFER_SIZE + 1];

  for (HttpServer::size_type size = server.size(); size > 0; --size) {

    if (server.is_listener(it) && it->revents == 0) {
      ++it;
      continue;
    }

    switch (it->revents) {

    case POLLPRI:
    case POLLIN:

      if (server.is_listener(it)) {

        it = server.create_connection();

      } else {

        readed = recv(it->fd, buffer, BUFFER_SIZE, 0);

        if (readed < 0) {
          it = server.close_connection(it);
          continue;

        } else if (!readed) {
          it = server.close_connection(it);
          continue;
        }

        buffer[readed] = 0;

        try {
          server.add_request(it->fd, buffer, readed);

        } catch (internals::BadRequestExcpt const &) {

          HttpResponseParser::send(
              it->fd, HttpResponseParser::set_error_response(
                          server.get_server_config(), internals::BadRequest));

          it = server.close_connection(it);
          continue;

        } catch (internals::EntityTooLargeExcpt const &) {

          std::string const &response = HttpResponseParser::set_error_response(
              server.get_server_config(), internals::EntityTooLarge);

          HttpResponseParser::send(it->fd, response);

          it = server.close_connection(it);
          continue;

        } catch (internals::IncompletedRequest const &) {
          break;
        }

        it->events = POLLOUT;
      }

      break;

    case POLLWRBAND:
    case POLLOUT: {
      std::string const &response = HttpResponseParser::build(it->fd, server);

      sent = HttpResponseParser::send(it->fd, response);

      if (sent < 0) {
        std::cerr << "[ERROR]: send error" << std::endl;
        it = server.close_connection(it);
        continue;
      }

      server.clear_requests(it->fd);
    }

      it->events = POLLIN;
      break;

    default:

      if (!it->revents)
        break;

      std::cerr << "[ERROR]: Some problem with the socket" << std::endl;
      it = server.close_connection(it);
      continue;
    }
    ++it;
  }
}

int main(int argc, char *argv[]) {
  ConfigFileParser parser;
  std::vector<HttpServer *> servers;
  std::vector<HttpServer *>::iterator _server;
  int status;

  if (argc != 2) {
    std::cerr << "[ERROR]: Wrong Number of Parameters" << std::endl;
    return 1;
  }

  if (parser.parser(argv[1])) {
    std::cerr << "[ERROR]: Wrong Configuration File" << std::endl;
    return 1;
  }

  if (init_servers_config(parser, servers)) {
    std::cerr << "[ERROR] Wrong Setup servers config" << std::endl;
    return 1;
  }

  while (true) {

    for (_server = servers.begin(); _server != servers.end(); ++_server) {
      HttpServer *server = *_server;
      status = poll(server->get_data(), server->size(), 0);

      if (status < 0) {
        if (errno == EINTR)
          continue;

        std::perror("[ERROR]: Pool: ");
        clear_http_servers(servers);
        return 1;
      }

      scan_sockets(*server);
    }
  }
  return 0;
}

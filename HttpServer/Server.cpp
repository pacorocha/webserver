#include "Server.hpp"
#include "Utils.hpp"
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <netdb.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

using namespace internals;

SetupListenerError::SetupListenerError() : msg("[ERROR] SetupListenerError") {}
SetupListenerError::SetupListenerError(char const *const msg) : msg(msg) {}
char const *SetupListenerError::what() const throw() { return msg; }

PollFd::PollFd() : pollfd() {}
PollFd::PollFd(int fd, short events) : pollfd() {
  this->fd = fd;
  this->events = events;
}

using namespace web;

Server::Server(Server const &server)
    : pollfds(server.pollfds), max_backlog(server.max_backlog) {}

Server::Server(int port) throw(SetupListenerError) : max_backlog(20) {

  struct addrinfo addinfo, *res, *hold;
  int status, listener, ok = 1;
  std::string port_as_string(int_to_string(port));

  memset(&addinfo, 0, sizeof addinfo);
  addinfo.ai_family = AF_INET;
  addinfo.ai_socktype = SOCK_STREAM;
  addinfo.ai_flags = AI_PASSIVE;

  status = getaddrinfo(NULL, port_as_string.c_str(), &addinfo, &res);

  if (status < 0) {
    throw SetupListenerError(gai_strerror(status));
  }

  for (hold = res; hold != NULL; hold = hold->ai_next) {
    listener = socket(hold->ai_family, hold->ai_socktype, hold->ai_protocol);

    if (listener < 0) {
      std::cerr << "[ERROR] SocketError: " << std::strerror(errno) << std::endl;
      continue;
    }

    fcntl(listener, F_SETFL, O_NONBLOCK, FD_CLOEXEC);

    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &ok, sizeof ok) < 0) {
      close(listener);
      freeaddrinfo(res);
      throw SetupListenerError("[ERROR] SetSockOpt: ");
    }

    if (bind(listener, hold->ai_addr, hold->ai_addrlen) < 0) {
      close(listener);
      std::cerr << "[ERROR] BindError: " << std::strerror(errno) << std::endl;
    }

    break;
  }

  freeaddrinfo(res);

  if (!hold)
    throw SetupListenerError("[ERROR] Couldn't find socket available");

  if (listen(listener, max_backlog) < 0) {
    close(listener);
    throw SetupListenerError("[ERROR] ListenerError: ");
  }

  pollfds.push_back(static_cast<struct pollfd>(PollFd(listener, POLLIN)));
}

Server::~Server() {
  std::vector<struct pollfd>::reverse_iterator it;

  for (it = pollfds.rbegin(); it != pollfds.rend(); ++it)
    close_connection(it.base());
}

Server::iterator Server::create_connection() {
  int newfd;

  newfd = accept(pollfds[0].fd, NULL, NULL);

  if (newfd < 0)
    std::cerr << "[ERROR] Accept: " << std::strerror(errno) << std::endl;
  else
    pollfds.push_back(static_cast<struct pollfd>(PollFd(newfd, POLLIN)));

  return pollfds.begin();
}

Server::iterator Server::close_connection(Server::iterator const &pos) {
  close(pos->fd);
  return pollfds.erase(pos);
}

Server::pointer Server::get_data() { return pollfds.data(); }
Server::const_iterator Server::begin() const { return pollfds.begin(); }
Server::iterator Server::begin() { return pollfds.begin(); }
Server::iterator Server::end() { return pollfds.end(); }
Server::const_iterator Server::end() const { return pollfds.end(); }
Server::size_type Server::size() const { return pollfds.size(); }
bool Server::is_listener(Server::iterator const &pos) const {
  return pos->fd == begin()->fd;
}

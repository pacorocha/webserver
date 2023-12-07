#ifndef SERVER_HPP
#define SERVER_HPP

#include <exception>
#include <poll.h>
#include <string>
#include <sys/poll.h>
#include <vector>

namespace internals {
class SetupListenerError : public std::exception {
public:
  SetupListenerError();
  SetupListenerError(char const *const msg);
  virtual char const *what() const throw();

  char const *const msg;
};

class PollFd : public pollfd {
public:
  PollFd();
  PollFd(int fd, short events);
};
}; // namespace internals

namespace web {

class Server {

  typedef std::vector<struct pollfd> vectorFds;
  typedef vectorFds::const_iterator const_iterator;

public:
  typedef vectorFds::iterator iterator;
  typedef vectorFds::size_type size_type;
  typedef vectorFds::pointer pointer;

  Server(int port) throw(internals::SetupListenerError);
  Server(Server const &server);
  virtual ~Server();

  size_type size() const;
  iterator begin();
  iterator end();
  iterator create_connection();

  pointer get_data();
  iterator close_connection(iterator const &pos);
  bool is_listener(iterator const &pos) const;

private:
  std::vector<struct pollfd> pollfds;
  int max_backlog;

  const_iterator begin() const;
  const_iterator end() const;
};

}; // namespace web

#endif // !SERVER_HPP

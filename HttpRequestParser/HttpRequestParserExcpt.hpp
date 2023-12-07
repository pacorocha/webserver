#ifndef HTTPREQUESTPARSEREXCEPT_HPP
#define HTTPREQUESTPARSEREXCEPT_HPP

#include <exception>

namespace internals {

class BadRequestExcpt : public std::exception {
public:
  BadRequestExcpt();
  virtual char const *what() const throw();
};

class IncompletedRequest : public std::exception {
public:
  IncompletedRequest();
  virtual char const *what() const throw();
};

class KeyError : public std::exception {
public:
  KeyError();
  virtual char const *what() const throw();
};

class EntityTooLargeExcpt : public std::exception {
public:
  EntityTooLargeExcpt();
  virtual char const *what() const throw();
};

}; // namespace internals

#endif // !HTTP_REQUEST_PARSER_EXCEPT_HPP

#ifndef HTTPRESPONSEPARSEREXCEPT_HPP
#define HTTPRESPONSEPARSEREXCEPT_HPP

#include <exception>
namespace internals {

class CommonException: public std::exception {
public:
  CommonException(int code);
  virtual char const *what() const throw() = 0;
  int code;
};

class NotFoundValidIndex : public CommonException {
public:
  NotFoundValidIndex();
  virtual char const *what() const throw();
};

class NotFoundExcept : public CommonException {
public:
  NotFoundExcept();
  virtual char const *what() const throw();
};

class NoPermissionToRead : public CommonException {
public:
  NoPermissionToRead();
  virtual char const *what() const throw();
};

}

#endif // !HTTPRESPONSEPARSEREXCEPT_HPP

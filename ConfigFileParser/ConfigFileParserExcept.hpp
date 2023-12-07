#ifndef CONFIGFILEPARSEREXCEPT_HPP
#define CONFIGFILEPARSEREXCEPT_HPP

#include <exception>
namespace internals {

class BadFormat : public std::exception {
public:
  BadFormat();
  virtual char const *what() const throw();
};

class InvalidModule : public std::exception {
public:
  InvalidModule();
  virtual char const *what() const throw();
};

class LocationNotFound : public std::exception {
public:
  LocationNotFound();
  virtual char const *what() const throw();
};

} // namespace internals
#endif // !DEBUG

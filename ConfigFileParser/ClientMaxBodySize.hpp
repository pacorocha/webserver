#ifndef CLIENTMAXBODYSIZE_HPP
#define CLIENTMAXBODYSIZE_HPP

#include "ConfigFileParserExcept.hpp"
#include <istream>

namespace internals {

class ClientMaxBodySize {
public:
  ClientMaxBodySize();
  ~ClientMaxBodySize();
  ClientMaxBodySize(std::istream &file) throw(BadFormat);

  ClientMaxBodySize &operator=(ClientMaxBodySize const &other);

  int get_size() const;
  char get_unity() const;

private:
  int _size;
  char _unity;

  bool _is_valid_unity(char unity) const;
};

}

#endif // !CLIENTMAXBODYSIZE_HPP

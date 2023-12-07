#ifndef RETURN_HPP
#define RETURN_HPP

#include "ConfigFileParserExcept.hpp"
#include <istream>
#include <string>
namespace internals {

class Return {
public:
  Return();
  Return(std::istream &file) throw(BadFormat);
  ~Return();

  int get_code() const;
  std::string const &get_content() const;
  bool is_redirect() const;

private:
  int _code;
  std::string _text_or_url;
};

} // namespace internals

#endif // !RETURN_HPP

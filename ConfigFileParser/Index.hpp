#ifndef INDEX_HPP
#define INDEX_HPP

#include "ConfigFileParserExcept.hpp"
#include <istream>
#include <string>
#include <vector>

namespace internals {

class Index {
public:
  Index();
  Index(std::istream &file) throw(BadFormat);
  ~Index();

  typedef std::vector<std::string>::const_iterator iterator;

  iterator begin() const;
  iterator end() const;

private:
  std::vector<std::string> _files;

  bool _is_valid_character_to_unix_file_name(char c) const;
}; // Index

} // namespace internals

#endif // !INDEX_HPP

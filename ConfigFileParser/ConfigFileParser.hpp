#ifndef CONFIGFILEPARSER_HPP
#define CONFIGFILEPARSER_HPP

#include "ServersParser.hpp"
#include <vector>

namespace web {

class ConfigFileParser {
public:
  ConfigFileParser();
  ~ConfigFileParser();

  typedef std::vector<internals::ServersParser>::const_iterator iterator;

  int parser(char const file_path[]) throw();

  iterator begin() const;
  iterator end() const;

private:
  std::vector<internals::ServersParser> _servers;
};

} // namespace web

#endif // !CONFIGFILEPARSER_HPP

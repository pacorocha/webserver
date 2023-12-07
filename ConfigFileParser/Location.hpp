#ifndef LOCATION_HPP
#define LOCATION_HPP

#include "AllowedMethods.hpp"
#include "ClientMaxBodySize.hpp"
#include "ConfigFileParserExcept.hpp"
#include "ErrorPage.hpp"
#include "Index.hpp"
#include "Return.hpp"
#include <istream>
#include <map>
#include <string>
#include <vector>

namespace internals {

class Location {
public:
  Location();
  ~Location();
  Location(std::istream &file) throw(BadFormat);

  typedef std::vector<Index>::const_iterator indices_iterator;
  typedef Return const &return_type;

  std::string const &get_uri() const;
  std::string const &get_root() const;
  Location const &get_location(std::string const &uri) const;

  // AutoIndex
  bool is_autoindex_activated() const;

  // Return
  bool has_return() const;
  return_type get_return() const;

  // Indices
  bool has_indices() const;
  indices_iterator begin_indices() const;
  indices_iterator end_indices() const;

  // AllowedMethods
  bool is_method_allowed(std::string const &method) const;

private:
  AllowedMethods _allowed_methods;
  bool _autoindex;
  std::string _path;
  std::string _root;
  std::vector<Index> _indices;
  std::map<std::string const, Location> _locations;
  Return _return;

  bool _allowed_methods_already_set;
  bool _return_already_set;

  void _validate_path(std::string const &path_to_validate) const
      throw(BadFormat);
}; // Location

} // namespace internals

#endif // !LOCATION_HPP

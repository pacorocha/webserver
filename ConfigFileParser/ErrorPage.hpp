#ifndef ERRORPAGE_HPP
#define ERRORPAGE_HPP

#include <fstream>
#include <istream>
#include <string>
#include <vector>

namespace internals {

class ErrorPage {
public:
  ErrorPage();
  ErrorPage(std::istream &file);
  ~ErrorPage();

  std::string const &get_page() const;
  bool search(int code) const;

private:
  std::vector<int> _codes;
  std::string _page;
}; // ErrorPage

} // namespace internals

#endif // !ERRORPAGE_HPP

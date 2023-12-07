#ifndef DELETERESPONSEPARSER_HPP
#define DELETERESPONSEPARSER_HPP

#include "HttpRequestParser.hpp"
#include "ServersParser.hpp"
#include <string>
namespace internals {

class DeleteResponse {
public:
  DeleteResponse();
  ~DeleteResponse();

  static std::string build(web::HttpRequestParser const &request,
                           ServersParser const &server);
};

} // namespace internals

#endif // !DELETERESPONSEPARSER_HPP

#include "DeleteResponseParser.hpp"
#include "GetResponseParser.hpp"
#include "HttpRequestParser.hpp"
#include "HttpResponseParser.hpp"
#include "HttpResponseParserExcept.hpp"
#include "Location.hpp"
#include <sstream>
#include <unistd.h>

using namespace internals;

DeleteResponse::DeleteResponse() {}
DeleteResponse::~DeleteResponse() {}

std::string DeleteResponse::build(const web::HttpRequestParser &request,
                                  const ServersParser &server) {
  std::stringstream stream;
  Location const *location;
  std::string path, uri = request.get("request-uri");

  location = web::HttpResponseParser::match_precise(request.get("request-uri"),
                                                    server);

  if (location) {
    path = web::HttpResponseParser::get_path(uri, location->get_root());

    if (web::HttpResponseParser::is_dir(path)) {
      stream << GetResponseParser::setup_response(
          GetResponseParser::get_error_page(server, Forbidden), ".html",
          Forbidden);

      return stream.str();
    } else {
      try {
        web::HttpResponseParser::check_access(path, F_OK);
        if (remove(path.c_str()) < 0) {

          stream << GetResponseParser::setup_response(
              GetResponseParser::get_error_page(server, ServerError), ".html",
              ServerError);
          return stream.str();
        };

      } catch (internals::NotFoundExcept const &) {
        stream << GetResponseParser::setup_response(
            GetResponseParser::get_error_page(server, NotFound), ".html",
            NotFound);
        return stream.str();
      }
    }

    stream << GetResponseParser::setup_response("", "", OK);
  } else {
    stream << GetResponseParser::setup_response(
        GetResponseParser::get_error_page(server, Forbidden), ".html",
        Forbidden);
  }

  return stream.str();
}

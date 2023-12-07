#include "HttpResponseParserExcept.hpp"
#include "HttpResponseParser.hpp"

using namespace internals;

CommonException::CommonException(int code) : code(code) {}

NotFoundValidIndex::NotFoundValidIndex() : CommonException(Forbidden) {}
char const *NotFoundValidIndex::what() const throw() {
  return (
    "<!DOCTYPE html><html><body><p><b>403 Forbidden !!!</b></p></body></html>"
  );
}


NotFoundExcept::NotFoundExcept() : CommonException(NotFound) {}
char const *NotFoundExcept::what() const throw() {
  return (
    "<!DOCTYPE html><html><body><p><b>404 Not Found !!!</b></p></body></html>"
  );
}

NoPermissionToRead::NoPermissionToRead() : CommonException(Forbidden) {}
char const *NoPermissionToRead::what() const throw() {
  return (
    "<!DOCTYPE html><html><body><p><b>403 Forbidden !!!</b></p></body></html>"
  );
}

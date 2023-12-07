#include "HttpRequestParserExcpt.hpp"

using namespace internals;

BadRequestExcpt::BadRequestExcpt() {}
char const *BadRequestExcpt::what() const throw() {
  return "[ERROR]: BadRequest(400)";
}

IncompletedRequest::IncompletedRequest() {}
char const *IncompletedRequest::what() const throw() {
  return "IncompletedRequest";
}

KeyError::KeyError() {}
char const *KeyError::what() const throw() { return "Key not found"; }

EntityTooLargeExcpt::EntityTooLargeExcpt() {}
char const *EntityTooLargeExcpt::what() const throw() {
  return "Entity Too Large";
}

#ifndef CGI_HPP
#define CGI_HPP

#include "HttpRequestParser.hpp"
#include "PostResponseParser.hpp"
#include "ServersParser.hpp"
#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

class CGI {
public:
  CGI(void);
  ~CGI(void);

  std::pair<int, std::string>
  processRequest(const web::HttpRequestParser &request,
                 internals::MultipartForm const &content_parsed,
                 std::string const &root,
                 internals::ServersParser const &server);

private:
  std::string findPhpBinaryInPath(std::string const &binary_name) const;

  std::vector<std::string> splitPath(const std::string &pathEnv) const;

  std::vector<std::string>
  buildEnvironmentVariables(const web::HttpRequestParser request,
                            internals::MultipartForm const &content_parsed,
                            std::string const &path) const;
  std::string getEnvironmentVariable(const std::string &variable) const;
  void handleCGIScript(std::stringstream &response);

  std::pair<int, std::string>
  executePHP(const web::HttpRequestParser request,
             internals::MultipartForm const &content_parsed,
             std::string const &root, std::string const &scriptName,
             std::string const &phpBinaryPath);
};

#endif

#include "CGI.hpp"
#include "PostResponseParser.hpp"
#include "ServersParser.hpp"
#include <cstddef>
#include <iostream>
#include <string>
#include <unistd.h>
#include <utility>
#include <vector>

CGI::CGI(void) {}
CGI::~CGI(void) {}

std::string CGI::getEnvironmentVariable(const std::string &variable) const {
  const char *envVar = std::getenv(variable.c_str());
  if (envVar != NULL) {
    return std::string(envVar);
  } else {
    return "";
  }
}

std::vector<std::string>
CGI::buildEnvironmentVariables(const web::HttpRequestParser request,
                               internals::MultipartForm const &content_parsed,
                               std::string const &path) const {
  std::vector<std::string> envVars;

  std::string const &requestMethod = request.get("Method");
  envVars.push_back("REQUEST_METHOD=" + requestMethod);

  std::string const &scriptName = request.get("Request-URI");
  envVars.push_back("SCRIPT_NAME=" + scriptName);

  envVars.push_back("FILE_NAME=" + content_parsed.file_name);

  envVars.push_back("PATH_INFO=" + path);

  if (requestMethod == "POST") {
    std::string const &contentType = content_parsed.content_type;
    envVars.push_back("CONTENT_TYPE=" + contentType);

    std::string const &contentLength = request.get("Content-Length");
    envVars.push_back("CONTENT_LENGTH=" + contentLength);
  }

  if (requestMethod == "GET")
    envVars.push_back("QUERY_STRING=" + request.get("query-string"));

  return envVars;
}

std::pair<int, std::string>
CGI::processRequest(const web::HttpRequestParser &request,
                    internals::MultipartForm const &content_parsed,
                    std::string const &root,
                    internals::ServersParser const &server) {
  std::string const &scriptName = root + request.get("Request-URI");

  if (scriptName.find(server.get_cgi_extension()) != std::string::npos) {
    std::string const &phpBinaryPath =
        findPhpBinaryInPath(server.get_cgi_binary());

    if (phpBinaryPath.empty()) {
      std::cerr << "[ERROR]: PHP interpreter not found in PATH." << std::endl;
      return std::make_pair(1, "");
    }

    return executePHP(request, content_parsed, root, scriptName, phpBinaryPath);
  }

  return std::make_pair(1, "");
}

std::pair<int, std::string>
CGI::executePHP(const web::HttpRequestParser request,
                internals::MultipartForm const &content_parsed,
                std::string const &root, std::string const &scriptName,
                std::string const &phpBinaryPath) {

  pid_t pid;
  int pipefd[2];
  int status;
  std::string content;
  std::string const &method = request.get("Method");

  if (pipe(pipefd) == -1) {
    std::cerr << "Error creating pipe." << std::endl;
  }

  switch (pid = fork()) {
  case -1:
    std::cerr << "Error forking process." << std::endl;
    close(pipefd[0]);
    close(pipefd[1]);
    break;

  case 0: {
    // Child process

    // Build argument list
    char *argv[] = {const_cast<char *>(phpBinaryPath.c_str()),
                    const_cast<char *>(scriptName.c_str()), NULL};

    // Build environment variable list
    std::vector<std::string> envVars =
        buildEnvironmentVariables(request, content_parsed, root);
    char *envp[envVars.size() + 1];
    for (size_t i = 0; i < envVars.size(); ++i) {
      envp[i] = const_cast<char *>(envVars[i].c_str());
    }
    envp[envVars.size()] = NULL;

    // Duplicate the write end of the pipe to stdout in the child process

    if (method == "GET")
      dup2(pipefd[1], STDOUT_FILENO);
    if (method == "POST")
      dup2(pipefd[0], STDIN_FILENO);

    close(pipefd[1]);
    close(pipefd[0]);
    // Close unused file descriptors

    status = execve(phpBinaryPath.c_str(), argv, envp);

    exit(status);
  }

  default:
    char buffer[501];
    int readed;
    // Parent process
    // Wait for the child process to finish

    if (method == "GET") {
      close(pipefd[1]);
      while ((readed = read(pipefd[0], buffer, 500)) > 0) {
        buffer[readed] = 0;
        content.append(buffer);
      }
      close(pipefd[0]);
    } else if (method == "POST") {
      close(pipefd[0]);
      write(pipefd[1], const_cast<char *>(content_parsed.content.c_str()),
            content_parsed.content.length());
      close(pipefd[1]);
    }

    waitpid(pid, &status, 0);

    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
      std::cerr << "[ERROR]: Error on execve" << std::endl;
      return std::make_pair(1, "");
    }
  }
  return std::make_pair(0, content);
}

std::string CGI::findPhpBinaryInPath(std::string const &binary_name) const {
  std::vector<std::string> const &pathSegments =
      splitPath(getEnvironmentVariable("PATH"));

  for (size_t i = 0; i < pathSegments.size(); ++i) {
    std::string const &potentialPhpBinary = pathSegments[i] + "/" + binary_name;
    if (access(potentialPhpBinary.c_str(), X_OK) == 0) {
      return potentialPhpBinary;
    }
  }

  return "";
}

std::vector<std::string> CGI::splitPath(const std::string &pathEnv) const {
  std::vector<std::string> pathSegments;
  std::istringstream pathStream(pathEnv);

  while (!pathStream.eof()) {
    std::string pathSegment;
    std::getline(pathStream, pathSegment, ':');
    pathSegments.push_back(pathSegment);
  }

  return pathSegments;
}

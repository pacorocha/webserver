#ifndef UTILS_HPP
#define UTILS_HPP

#include "ConfigFileParser.hpp"
#include "HttpServer.hpp"
#include <string>
#include <vector>

std::string int_to_string(int value);
int string_to_int(std::string const &value);

int init_servers_config(web::ConfigFileParser const &parser,
                        std::vector<web::HttpServer *> &servers_config);

void clear_http_servers(std::vector<web::HttpServer *> &servers_config);

#endif // !UTILS_HPP

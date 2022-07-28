#ifndef FFS_CURL_H
#define FFS_CURL_H

#include <string>
#include <sstream>
#include <vector>
#include <utility>

// Post to url with params. Returns JSON root object of response body
std::stringstream* post(std::string, std::string, std::vector<std::pair<std::string, std::string>> = {});
std::stringstream* get(std::string, std::string="");

#endif
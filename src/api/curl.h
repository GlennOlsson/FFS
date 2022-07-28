#ifndef FFS_CURL_H
#define FFS_CURL_H

#include <string>
#include <sstream>
#include <vector>
#include <utility>

std::stringstream* post(std::string url, std::string params="");
std::stringstream* get(std::string url, std::string params="");

#endif
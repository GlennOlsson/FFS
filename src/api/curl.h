#ifndef FFS_CURL_H
#define FFS_CURL_H

#include <string>
#include <sstream>
#include <memory>

namespace FFS::API::HTTP {
	std::shared_ptr<std::stringstream> get(std::string url, std::string params="");
}

#endif
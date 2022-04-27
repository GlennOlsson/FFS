#pragma once

#include <string>
#include <map>
#include <vector>

namespace FFS {
class Directory {
private:
	/**
	 * @brief Map of (filename, inode id) describing the content of the directory
	 */
	std::map<std::string, int> entries;

public:
	/**
	 * @brief Get the content of the directory
	 * 
	 * @return std::vector<std::string> a list of all filenames 
	 */
	std::vector<std::string> content();
};
}
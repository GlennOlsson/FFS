#pragma once

#include "../helpers/types.h"

#include <string>
#include <map>
#include <vector>
#include <iostream>

namespace FFS {
class Directory {
public:
	/**
	 * @brief Map of (filename, inode id) describing the content of the directory. The filename is limited to 255 characters
	 */
	std::map<std::string, inode_id>* entries;

	Directory(std::map<std::string, inode_id>* entries, inode_id self_id);

	inode_id self_id;
	
	Directory(inode_id self_id);
	/**
	 * @brief Returns the size of the object in terms of bytes
	 * 
	 * @return uint32_t the amount of bytes occupied by object
	 */
	uint32_t size();

	/**
	 * @brief Sterilizes the directory into a stream as bytes 
	 * 
	 * @param stream the output stream
	 */
	void sterilize(std::ostream& stream);
	
	/**
	 * @brief Creates an instance of a Directory from an FFS image
	 * 
	 * @param stream the input stream for the FFS image
	 * @return Directory the instanciated directory
	 */
	static Directory* desterilize(std::istream& stream);

	/**
	 * @brief Get the content of the directory
	 * 
	 * @return std::vector<std::string> a list of all filenames 
	 */
	std::vector<std::string> content();

	// Create file in directory
	void add_entry(std::string name, inode_id id);
	// Get a file with specified name
	inode_id get_file(std::string name);

	/**
	 * @brief Compare equality of two directories
	 * 
	 * @param rhs the other directory to compare with
	 * @return true if the directories are equal
	 * @return false if the directories are unequal
	 */
	bool operator==(const Directory& rhs) const;
};
}
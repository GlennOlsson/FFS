#pragma once

#include "../helpers/types.h"

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <Magick++.h>

namespace FFS {
class Directory {
private:
	/**
	 * @brief Map of (filename, inode id) describing the content of the directory. The filename is limited to 255 characters
	 */
	std::map<std::string, inode_id>* entries;

	Directory(std::map<std::string, inode_id>* entries);

public:
	
	Directory();
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
	 * @brief Get a Blob representing the directory as a FFS image
	 * 
	 * @return Magick::Blob* 
	 */
	Magick::Blob* blob();

	static Directory* from_blob(Magick::Blob* blob);

	/**
	 * @brief Save directory as an FFS image to file at path
	 * 
	 * @param path the path to save the file to
	 */
	void save(std::string path);

	/**
	 * @brief Load a directory from an FFS image
	 * 
	 * @param path the path of the FFS image
	 * @return Directory* a pointer to the resulting directory
	 */
	static Directory* load(std::string path);

	/**
	 * @brief Get the content of the directory
	 * 
	 * @return std::vector<std::string> a list of all filenames 
	 */
	std::vector<std::string> content();

	// Create file in directory
	inode_id create_file(std::string name);
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
#pragma once

#include <string>
#include <map>
#include <vector>
#include <iostream>

namespace FFS {
class Directory {
private:
	/**
	 * @brief Map of (filename, inode id) describing the content of the directory. The filename is limited to 255 characters
	 */
	std::map<std::string, uint32_t>* entries;

	Directory(std::map<std::string, uint32_t>* entries);

public:
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
#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <unordered_map>

namespace FFS {

class InodeEntry {
private:
	//TODO: add more metadata
	// Total file length
	int length;
	// twitter id is 64 bit https://developer.twitter.com/en/docs/twitter-ids
	std::vector<unsigned long>* tweet_blocks;
	
public:
	InodeEntry(int length, std::vector<unsigned long>* tweet_blocks);

	/**
	 * @brief Returns the size of the object in terms of bytes
	 * 
	 * @return int the amount of bytes occupied by object
	 */
	int size();

	/**
	 * @brief Sterilizes the entry into the stream as bytes in the following manner
	 * byte 0-3: length attribute (int)
	 * byte 4-7: n = amount of tweet blocks (int)
	 * byte 8-...: 8 bytes per element, n elements (tweet_blocks attribute)
	 *
	 * 
	 * With 4 bytes representing the amount of blocks we can achieve a theoretical
	 * file limit of 6.8 * 10^10 mb == 64 Petabyte
	 * 
	 * @param stream the output stream
	 */
	void sterilize(std::ostream& stream);
	
	/**
	 * @brief Creates an instance of an Inode Entry from an FFS image
	 * 
	 * @param stream the input stream for the FFS image
	 * @return InodeEntry the instanciated table
	 */
	static InodeEntry* desterilize(std::istream& stream);

	bool operator==(const InodeEntry& rhs) const;
};

class InodeTable {
public:
	std::unordered_map<unsigned int, InodeEntry*>* entries;

	InodeTable(std::unordered_map<unsigned int, InodeEntry*>* entries);

	/**
	 * @brief Returns the size of the object in terms of bytes
	 * 
	 * @return int the amount of bytes occupied by object
	 */
	int size();

	/**
	 * @brief Sterilizes the entry into the stream as bytes in the following manner
	 * byte 0-3: n = amount of entries
	 * byte 4-...: n pairs of <id, sterlizied Inode Entries @see InodeEntry::sterilize>
	 * 		id is of 4 bytes
	 *
	 * @param stream the output stream
	 */
	void sterilize(std::ostream& stream);
	
	/**
	 * @brief Creates an instance of an Inode Table from an FFS image
	 * 
	 * @param stream the input stream for the FFS image
	 * @return InodeTable the instanciated table
	 */
	static InodeTable* desterilize(std::istream& stream);

	void save(std::string path);
	static InodeTable* load(std::string path);

	bool operator==(const InodeTable& rhs) const;
};
}
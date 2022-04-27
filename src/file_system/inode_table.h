#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <map>

namespace FFS {

class InodeTable {
private:
	class InodeEntry {
	public:
	//TODO: add more metadata
	// Total file length
	uint32_t length;
	// twitter id is 64 bit https://developer.twitter.com/en/docs/twitter-ids
	std::vector<uint64_t>* tweet_blocks;
	
	InodeEntry(uint32_t length, std::vector<uint64_t>* tweet_blocks);

	/**
	 * @brief Returns the size of the object in terms of bytes
	 * 
	 * @return uint32_t the amount of bytes occupied by object
	 */
	uint32_t size();

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

	// Inode -> Inode Entry
	std::map<uint32_t, InodeEntry*>* entries;

	InodeTable(std::map<uint32_t, InodeEntry*>* entries);

public:
	/**
	 * @brief Returns the size of the object in terms of bytes
	 * 
	 * @return uint32_t the amount of bytes occupied by object
	 */
	uint32_t size();

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
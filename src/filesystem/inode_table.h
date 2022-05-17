#pragma once

#include "../helpers/types.h"

#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <Magick++.h>

namespace FFS {

class InodeEntry {
private:
	//TODO: add more metadata
	// Total file length
	uint32_t length;
	uint8_t is_dir;
	std::vector<post_id>* post_blocks;
	
public:
	InodeEntry(uint32_t length, std::vector<post_id>* post_blocks, uint8_t is_dir);
	InodeEntry(uint32_t length, post_id post, uint8_t is_dir);
	/**
	 * @brief Returns the size of the object in terms of bytes
	 * 
	 * @return uint32_t the amount of bytes occupied by object
	 */
	uint32_t size();

	/**
	 * @brief Sterilizes the entry into the stream as bytes in the following manner
	 * byte 0-3: length attribute (int)
	 * byte 4-7: n = amount of post blocks (int)
	 * byte 8-...: 8 bytes per element, n elements (post_blocks attribute)
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

	friend class InodeTable;
};
class InodeTable {
private:

	// Inode -> Inode Entry
	std::map<inode_id, InodeEntry*>* entries;

	InodeTable(std::map<inode_id, InodeEntry*>* entries);

public:
	InodeTable();

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

	Magick::Blob* blob();
	static InodeTable* from_blob(Magick::Blob* blob);

	// TODO: Test these methods
	inode_id new_file(std::vector<post_id>* posts, uint32_t length, uint8_t is_dir);
	InodeEntry* entry(const inode_id& id);

	bool operator==(const InodeTable& rhs) const;
};
}
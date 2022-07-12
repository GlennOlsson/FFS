#pragma once

#include "../helpers/types.h"

#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <memory>

namespace FFS {

class InodeEntry {
public:
	//TODO: add more metadata
	// Total file length
	uint32_t length;
	uint8_t is_dir;
	
	InodeEntry(uint32_t length, std::shared_ptr<std::vector<post_id>> post_blocks, uint8_t is_dir);
	InodeEntry(uint32_t length, post_id post, uint8_t is_dir);
	~InodeEntry();

	std::shared_ptr<std::vector<post_id>> post_blocks;
	
	/**
	 * @brief Returns the size of the object in terms of bytes
	 * 
	 * @return uint32_t the amount of bytes occupied by object
	 */
	uint32_t size();

	/**
	 * @brief serializes the entry into the stream as bytes in the following manner
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
	void serialize(std::ostream& stream);
	
	/**
	 * @brief Creates an instance of an Inode Entry from an FFS image
	 * 
	 * @param stream the input stream for the FFS image
	 * @return InodeEntry the instanciated table
	 */
	static std::shared_ptr<InodeEntry> deserialize(std::istream& stream);

	bool operator==(const InodeEntry& rhs) const;

	friend class InodeTable;
};
class InodeTable {
public:
	// Inode -> Inode Entry
	std::shared_ptr<std::map<inode_id, std::shared_ptr<InodeEntry>>> entries;

	InodeTable(std::shared_ptr<std::map<inode_id,  std::shared_ptr<InodeEntry>>> entries);

	InodeTable();

	/**
	 * @brief Returns the size of the object in terms of bytes
	 * 
	 * @return uint32_t the amount of bytes occupied by object
	 */
	uint32_t size();

	/**
	 * @brief serializes the entry into the stream as bytes in the following manner
	 * byte 0-3: n = amount of entries
	 * byte 4-...: n pairs of <id, sterlizied Inode Entries @see InodeEntry::serialize>
	 * 		id is of 4 bytes
	 *
	 * @param stream the output stream
	 */
	void serialize(std::ostream& stream);
	
	/**
	 * @brief Creates an instance of an Inode Table from an FFS image
	 * 
	 * @param stream the input stream for the FFS image
	 * @return InodeTable the instanciated table
	 */
	static std::shared_ptr<InodeTable> deserialize(std::istream& stream);

	// TODO: Test these methods
	inode_id new_file(std::shared_ptr<std::vector<post_id>> posts, uint32_t length, uint8_t is_dir);
	std::shared_ptr<InodeEntry> entry(const inode_id& id);
	void remove_entry(inode_id);

	bool operator==(const InodeTable& rhs) const;
};
}
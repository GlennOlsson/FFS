#pragma once

#include "../helpers/types.h"

#include <vector>
#include <string>
#include <iostream>
#include <map>
#include <memory>
#include <chrono>

namespace FFS {

/**
* @brief Describes and entry in the inode table, representing a file or directory
*/
class InodeEntry {
public:
	/**
	* @brief The size of the file (not used for directories) 
	*/
	uint32_t length;

	/**
	* @brief True if the entry describes a directory, false if it describes a file
	*/
	uint8_t is_dir;

	// When the file first was created
	uint64_t time_created;
	// When the file last was accessed
	uint64_t time_accessed;
	// When the file last was modified
	uint64_t time_modified;
	
	/**
	* @brief A list representing the posts of the file or directory. 
	*/
	posts_t post_ids;

	InodeEntry(uint32_t length, posts_t post_ids, uint8_t is_dir);
	InodeEntry(uint32_t length, post_id_t post, uint8_t is_dir);
	~InodeEntry();

	/**
	 * @brief serializes the entry into the stream as bytes in the following manner
	 * byte 0-3: length attribute (int)
	 * byte 4-7: n = amount of post blocks (int)
	 * byte 8-...: 8 bytes per element, n elements (post_ids attribute)
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

/**
* @brief Describes the inode table of the filesystem. The table consists of multiple inode entries
*/
class InodeTable {
public:
	/**
	* @brief Map of (inode id, Inode entry) describing the content of the inode table
	*/
	std::shared_ptr<std::map<inode_t, std::shared_ptr<InodeEntry>>> entries;

	InodeTable(std::shared_ptr<std::map<inode_t,  std::shared_ptr<InodeEntry>>> entries);

	InodeTable();

	inode_t largest_inode();

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
	inode_t new_file(posts_t posts, uint32_t length, uint8_t is_dir);
	std::shared_ptr<InodeEntry> entry(const inode_t& id);
	void remove_entry(inode_t);

	bool operator==(const InodeTable& rhs) const;
};
}
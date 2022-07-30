#include "inode_table.h"

#include "../helpers/functions.h"
#include "../helpers/types.h"
#include "../helpers/constants.h"

#include "../system/state.h"

#include "../exceptions/exceptions.h"

#include "storage.h"
#include "file_coder.h"
#include "directory.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <Magick++.h>
#include <memory>
#include <chrono>

// Inode Entry...

FFS::InodeEntry::InodeEntry(uint32_t length, std::shared_ptr<std::vector<post_id>> post_blocks, uint8_t is_dir = false) {
	this->length = length;
	this->is_dir = is_dir;
	this->post_blocks = std::move(post_blocks);
	if(this->post_blocks == nullptr) {
		this->post_blocks = std::make_shared<std::vector<post_id>>();
	}

	// Just created, so set as current time
	auto now = std::chrono::system_clock::now().time_since_epoch();
	this->time_created = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
	this->time_accessed = this->time_created;
	this->time_modified = this->time_created;
}

FFS::InodeEntry::~InodeEntry() {
	this->post_blocks.reset();
}

uint32_t FFS::InodeEntry::size() {
	uint32_t value = 0;
	value += sizeof(uint32_t); // length
	value += sizeof(uint8_t); // is_dir
	value += sizeof(uint32_t); // amount of post blocks
	value += this->post_blocks->size() * sizeof(uint64_t); // per post block

	return value;
}

void FFS::InodeEntry::serialize(std::ostream& stream) {
	FFS::write_i(stream, this->length);
	FFS::write_c(stream, this->is_dir);
	FFS::write_l(stream, this->time_created);
	FFS::write_l(stream, this->time_accessed);
	FFS::write_l(stream, this->time_modified);

	std::cout << "Ser inode entry, is_dir: " << (this->is_dir ? "yes" : "no") << std::endl;
	std::cout << "Ser inode entry, post_blocks nullptr: " << (this->post_blocks == nullptr ? "yes" : "no") << std::endl;

	// If has no posts, just write 0 and return
	if(this->post_blocks == nullptr) {
		FFS::write_i(stream, 0);
		return;
	}

	std::cout << "Ser inode entry, post_blocks size: " << (this->post_blocks->size()) << std::endl;

	FFS::write_i(stream, this->post_blocks->size());
	for (post_id entry : *post_blocks) {
		FFS::write_str(stream, entry);
	}
}

std::shared_ptr<FFS::InodeEntry> FFS::InodeEntry::deserialize(std::istream& stream) {
	uint32_t length, block_count;
	uint64_t time_created, time_accessed, time_modified;
	uint8_t is_dir;

	FFS::read_i(stream, length);
	FFS::read_c(stream, is_dir);
	FFS::read_l(stream, time_created);
	FFS::read_l(stream, time_accessed);
	FFS::read_l(stream, time_modified);
	
	FFS::read_i(stream, block_count);
	auto blocks = std::make_shared<std::vector<post_id>>();
	std::cout << "DESER I_ENTRY BLOCK COUNT: " << block_count << std::endl;
	while (block_count > 0) {
		std::cout << "Deser inode entry " << (is_dir ? "dir" : "file") << std::endl;
		post_id id;
		FFS::read_str(stream, id);

		std::cout << "Post_id = \"" << id << "\"" << std::endl;

		blocks->push_back(id);
		block_count--;
	}

	auto entry = std::make_shared<InodeEntry>(length, blocks, is_dir);

	entry->time_created = time_created;
	entry->time_accessed = time_accessed;
	entry->time_modified = time_modified;
	
	return entry;
}

bool FFS::InodeEntry::operator==(const FFS::InodeEntry& rhs) const {
	// Equal if same amount of blocks, and the blocks array is equal (order
	// matters!!)
	return this->length == rhs.length &&
		   (*this->post_blocks) == (*rhs.post_blocks);
}

// Inode Table...

FFS::InodeTable::InodeTable(std::shared_ptr<std::map<uint32_t, std::shared_ptr<FFS::InodeEntry>>> entries) {
	this->entries = std::move(entries);
}

// Creates empty inode table with only a root directory
FFS::InodeTable::InodeTable() {
	std::shared_ptr<std::map<inode_id, std::shared_ptr<InodeEntry>>> empty_entries = std::make_shared<std::map<inode_id, std::shared_ptr<InodeEntry>>>();

	// Create root dir entry of base size, but no entries
	std::shared_ptr<std::vector<post_id>> no_posts = nullptr;
	auto entry = std::make_shared<InodeEntry>(0, no_posts, true);
	// Root dir should specific inode id
	empty_entries->insert({FFS_ROOT_INODE, std::move(entry)});

	// TODO: Update entries with root dir and its id
	this->entries = std::move(empty_entries);
}

uint32_t FFS::InodeTable::size() {
	uint32_t size = 4;  // 4 bytes for amount of entries
	for (auto entry : *this->entries) {
		size += 4;					   // Size of id, int
		size += entry.second->size();  // Add the size for each entry
	}

	return size;
}

void FFS::InodeTable::serialize(std::ostream& stream) {
	uint32_t total_entries = this->entries->size();

	std::cout << "serializing inode table, entries: " << total_entries << std::endl;

	FFS::write_i(stream, total_entries);

	// For each entry add its id, and the serialized entry
	for (auto entry : *this->entries) {
		inode_id id = entry.first;
		FFS::write_i(stream, id);

		entry.second->serialize(stream);
	}
}

std::shared_ptr<FFS::InodeTable> FFS::InodeTable::deserialize(std::istream& stream) {
	uint32_t entries_count;

	FFS::read_i(stream, entries_count);

	auto entries = std::make_shared<std::map< inode_id, std::shared_ptr<FFS::InodeEntry>>>();

	while(entries_count--) {
		inode_id id;

		FFS::read_i(stream, id);

		auto entry = InodeEntry::deserialize(stream);
		entries->insert({id, entry});
	}

	return std::make_shared<InodeTable>(entries);
}

FFS::inode_id FFS::InodeTable::new_file(std::shared_ptr<std::vector<FFS::post_id>> posts, uint32_t length, uint8_t is_dir) {
	auto entry = std::make_shared<InodeEntry>(length, posts, is_dir);
	
	// Find next inode id to use
	inode_id new_id;
	if(this->entries->empty()) {
		new_id = 0;
	} else {
		auto r_it = this->entries->rbegin();
		inode_id biggest_id = r_it->first;
		new_id = biggest_id + 1;
	}

	// Use new id as inode id, and return it
	this->entries->insert({new_id, entry});

	return new_id;
}

std::shared_ptr<FFS::InodeEntry> FFS::InodeTable::entry(const FFS::inode_id& id) {
	if(this->entries->contains(id))
		return this->entries->at(id);
	
	throw FFS::NoFileWithInode(id);
}

void FFS::InodeTable::remove_entry(FFS::inode_id id) {
	auto entry = this->entries->at(id);
	entry.reset();
	entries->erase(id);
}

bool FFS::InodeTable::operator==(const FFS::InodeTable& rhs) const {
	// Compare size of tables (maps), and compare content of maps
	return this->entries->size() == rhs.entries->size() &&
		   std::equal(this->entries->begin(), this->entries->end(),
					  rhs.entries->begin(), [](auto e1, auto e2) {
						  // Compare keys, and de-reference InodeTable pointers
						  // and compare
						  return e1.first == e2.first &&
								 *e1.second == *e2.second;
					  });
}
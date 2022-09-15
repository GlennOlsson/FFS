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

FFS::InodeEntry::InodeEntry(uint32_t length, posts_t post_ids, uint8_t is_dir = false) {
	this->length = length;
	this->is_dir = is_dir;
	this->post_ids = std::move(post_ids);
	if(this->post_ids == nullptr) {
		this->post_ids = std::make_shared<std::vector<post_id_t>>();
	}

	// Just created, so set as current time
	auto now = FFS::curr_milliseconds();
	this->time_created = now;
	this->time_accessed = now;
	this->time_modified = now;
}

FFS::InodeEntry::~InodeEntry() {
	this->post_ids.reset();
}

void FFS::InodeEntry::serialize(std::ostream& stream) {
	FFS::write_i(stream, this->length); // 4
	FFS::write_c(stream, this->is_dir); // 1
	FFS::write_l(stream, this->time_created); // 8
	FFS::write_l(stream, this->time_accessed); // 8
	FFS::write_l(stream, this->time_modified); // 8

	// If has no posts, just write 0 and return
	if(this->post_ids == nullptr) {
		FFS::write_i(stream, 0); // 4
		return;
	}

	FFS::write_i(stream, this->post_ids->size()); // 4
	for (post_id_t entry : *post_ids) { // Variable
		FFS::write_str(stream, entry); // Variable, often 12
	}

	// Total: 33 + 12*posts, 33, 45, ...
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
	auto blocks = std::make_shared<std::vector<post_id_t>>();
	while (block_count > 0) {
		post_id_t id;
		FFS::read_str(stream, id);

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
		   (*this->post_ids) == (*rhs.post_ids);
}

// Inode Table...

FFS::InodeTable::InodeTable(std::shared_ptr<std::map<uint32_t, std::shared_ptr<FFS::InodeEntry>>> entries) {
	this->entries = std::move(entries);
}

// Creates empty inode table with only a root directory
FFS::InodeTable::InodeTable() {
	std::shared_ptr<std::map<inode_t, std::shared_ptr<InodeEntry>>> empty_entries = std::make_shared<std::map<inode_t, std::shared_ptr<InodeEntry>>>();

	// Create root dir entry of base size, but no entries
	posts_t no_posts = nullptr;
	auto entry = std::make_shared<InodeEntry>(0, no_posts, true);
	// Root dir should specific inode id
	empty_entries->insert({FFS_ROOT_INODE, std::move(entry)});

	this->entries = std::move(empty_entries);
}

void FFS::InodeTable::serialize(std::ostream& stream) {
	uint32_t total_entries = this->entries->size();

	FFS::write_c(stream, 'i');
	FFS::write_c(stream, 'n');
	FFS::write_c(stream, 'd');

	FFS::write_i(stream, total_entries);

	// For each entry add its id, and the serialized entry
	for (auto entry : *this->entries) {
		inode_t id = entry.first;
		FFS::write_i(stream, id);

		entry.second->serialize(stream);
	}
}

std::shared_ptr<FFS::InodeTable> FFS::InodeTable::deserialize(std::istream& stream) {
	uint32_t entries_count;

	char c1, c2, c3;
	FFS::read_c(stream, c1);
	FFS::read_c(stream, c2);
	FFS::read_c(stream, c3);
	if(c1 != 'i' || c2 != 'n' || c3 != 'd') {
		throw FFS::FFSFileNotInodeTable();
	}

	FFS::read_i(stream, entries_count);

	auto entries = std::make_shared<std::map< inode_t, std::shared_ptr<FFS::InodeEntry>>>();

	while(entries_count--) {
		inode_t id;

		FFS::read_i(stream, id);

		auto entry = InodeEntry::deserialize(stream);
		entries->insert({id, entry});
	}

	return std::make_shared<InodeTable>(entries);
}

FFS::inode_t FFS::InodeTable::new_file(posts_t posts, uint32_t length, uint8_t is_dir) {
	auto entry = std::make_shared<InodeEntry>(length, posts, is_dir);
	
	// Find next inode id to use
	inode_t new_id;
	if(this->entries->empty()) {
		new_id = 0;
	} else {
		auto r_it = this->entries->rbegin();
		inode_t biggest_id = r_it->first;
		new_id = biggest_id + 1;
	}

	// Use new id as inode id, and return it
	this->entries->insert({new_id, entry});

	return new_id;
}

std::shared_ptr<FFS::InodeEntry> FFS::InodeTable::entry(const FFS::inode_t& id) {
	if(this->entries->contains(id)) {
		this->entries->at(id)->time_accessed = FFS::curr_milliseconds();
		return this->entries->at(id);
	}
	
	throw FFS::NoFileWithInode(id);
}

void FFS::InodeTable::remove_entry(FFS::inode_t id) {
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
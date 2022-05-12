#include "inode_table.h"

#include "../helpers/functions.h"
#include "../helpers/types.h"

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

// Inode Entry...

FFS::InodeEntry::InodeEntry(uint32_t length,
							std::vector<post_id>* post_blocks) {
	this->length = length;
	this->post_blocks = post_blocks;
}

uint32_t FFS::InodeEntry::size() {
	uint32_t value = 0;
	value += 4;								  // Length field, int = 4 bytes
	value += 4;								  // Amount of entries = 4 bytes
	value += this->post_blocks->size() * 8;  // 8 bytes per element

	return value;
}

void FFS::InodeEntry::sterilize(std::ostream& stream) {
	FFS::write_i(stream, this->length);
	FFS::write_i(stream, this->post_blocks->size());
	for (post_id entry : *post_blocks) {
		FFS::write_l(stream, entry);
	}
}

FFS::InodeEntry* FFS::InodeEntry::desterilize(std::istream& stream) {
	uint32_t length, block_count;

	FFS::read_i(stream, length);
	FFS::read_i(stream, block_count);

	std::vector<post_id>* blocks = new std::vector<post_id>();
	while (block_count-- > 0) {
		post_id l;
		FFS::read_l(stream, l);

		blocks->push_back(l);
	}

	return new InodeEntry(length, blocks);
}

bool FFS::InodeEntry::operator==(const FFS::InodeEntry& rhs) const {
	// Equal if same amount of blocks, and the blocks array is equal (order
	// matters!!)
	return this->length == rhs.length &&
		   (*this->post_blocks) == (*rhs.post_blocks);
}

// Inode Table...

FFS::InodeTable::InodeTable(std::map< uint32_t, FFS::InodeEntry*>* entries) {
	this->entries = entries;
}

FFS::InodeTable::InodeTable() {
	std::map<uint32_t,InodeEntry*>* empty_entries = new std::map<uint32_t,InodeEntry*>();

	// Directory* root_dir = new Directory();
	// std::vector<post_id> ids = FFS::Storage::upload_file(root_dir);
	// TODO: Update entries with root dir and its id
	this->entries = empty_entries;
}

uint32_t FFS::InodeTable::size() {
	uint32_t size = 4;  // 4 bytes for amount of entries
	for (auto entry : *this->entries) {
		size += 4;					   // Size of id, int
		size += entry.second->size();  // Add the size for each entry
	}

	return size;
}

void FFS::InodeTable::sterilize(std::ostream& stream) {
	uint32_t total_entries = this->entries->size();

	FFS::write_i(stream, total_entries);

	// For each entry add its id, and the sterilized entry
	for (auto entry : *this->entries) {
		inode_id id = entry.first;
		FFS::write_i(stream, id);

		entry.second->sterilize(stream);
	}
}

FFS::InodeTable* FFS::InodeTable::desterilize(std::istream& stream) {
	uint32_t entries_count;

	FFS::read_i(stream, entries_count);

	auto entries = new std::map< inode_id, FFS::InodeEntry*>();

	while(entries_count--) {
		inode_id id;

		FFS::read_i(stream, id);

		InodeEntry* entry = InodeEntry::desterilize(stream);
		entries->insert({id, entry});
	}

	return new InodeTable(entries);
}

void FFS::InodeTable::save(std::string path) {
	std::stringbuf buf;
	std::basic_iostream stream(&buf);

	this->sterilize(stream);

	uint32_t size = this->size();
	_save_encoded_image(path, stream, size);
}

FFS::InodeTable* FFS::InodeTable::load(std::string path) {
	std::stringbuf buf;
	std::basic_iostream stream(&buf);

	_read_encoded_image({path}, stream);

	return desterilize(stream);
}

FFS::inode_id FFS::InodeTable::new_file(std::vector<FFS::post_id>* posts, uint32_t length) {
	InodeEntry* entry = new InodeEntry(length, posts);
	inode_id new_id;
	if(this->entries->empty()) {
		new_id = 0;
	} else {
		auto r_it = this->entries->rbegin();
		inode_id biggest_id = r_it->first;
		new_id = biggest_id + 1;
	}
	this->entries->insert({new_id, entry});

	return new_id;
}

FFS::InodeEntry* FFS::InodeTable::entry(const FFS::inode_id& id) {
	return this->entries->at(id);
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
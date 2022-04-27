#include "inode_table.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "../helpers/functions.h"
#include "file_coder.h"
// Inode Entry...

FFS::InodeEntry::InodeEntry(uint32_t length,
							std::vector< uint64_t>* tweet_blocks) {
	this->length = length;
	this->tweet_blocks = tweet_blocks;
}

uint32_t FFS::InodeEntry::size() {
	uint32_t value = 0;
	value += 4;								  // Length field, int = 4 bytes
	value += 4;								  // Amount of entries = 4 bytes
	value += this->tweet_blocks->size() * 8;  // 8 bytes per element

	return value;
}

void FFS::InodeEntry::sterilize(std::ostream& stream) {
	FFS::write_i(stream, this->length);
	FFS::write_i(stream, this->tweet_blocks->size());
	for (uint64_t entry : *tweet_blocks) {
		FFS::write_l(stream, entry);
	}
}

FFS::InodeEntry* FFS::InodeEntry::desterilize(std::istream& stream) {
	uint32_t length, block_count;

	FFS::read_i(stream, length);
	FFS::read_i(stream, block_count);

	std::vector<uint64_t>* blocks = new std::vector<uint64_t>();
	while (block_count-- > 0) {
		uint64_t signed_l;
		FFS::read_l(stream, signed_l);

		uint64_t l = signed_l;

		blocks->push_back(l);
	}

	return new InodeEntry(length, blocks);
}

bool FFS::InodeEntry::operator==(const FFS::InodeEntry& rhs) const {
	// Equal if same amount of blocks, and the blocks array is equal (order
	// matters!!)
	return this->length == rhs.length &&
		   (*this->tweet_blocks) == (*rhs.tweet_blocks);
}

// Inode Table...

FFS::InodeTable::InodeTable(std::map< uint32_t, FFS::InodeEntry*>* entries) {
	this->entries = entries;
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
		uint32_t id = entry.first;
		FFS::write_i(stream, id);

		entry.second->sterilize(stream);
	}
}

FFS::InodeTable* FFS::InodeTable::desterilize(std::istream& stream) {
	uint32_t entries_count;

	FFS::read_i(stream, entries_count);

	auto entries = new std::map< uint32_t, FFS::InodeEntry*>();

	while(entries_count--) {
		uint32_t signed_id;

		FFS::read_i(stream, signed_id);

		uint32_t id = signed_id;

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
	create_image(path, stream, size);
}

FFS::InodeTable* FFS::InodeTable::load(std::string path) {
	std::stringbuf buf;
	std::basic_iostream stream(&buf);

	decode({path}, stream);

	return desterilize(stream);
	;
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

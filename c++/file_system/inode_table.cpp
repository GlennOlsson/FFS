#include "inode_table.h"
#include "file_coder.h"

#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>

// Inode Entry...

FFS::InodeEntry::InodeEntry(int length, std::vector<unsigned long>& tweet_blocks) {
	this->length = length;
	this->tweet_blocks = tweet_blocks;
}

FFS::InodeEntry::InodeEntry(const InodeEntry &entry) {
	this->length = entry.length;
	this->tweet_blocks = std::vector<unsigned long>(entry.tweet_blocks);
}

int FFS::InodeEntry::size() {
	int value = 0;
	value += 4; // Length field, int = 4 bytes
	value += 4; // Amount of entries = 4 bytes
	value += this->tweet_blocks.size() * 8; // 8 bytes per element
	
	return value;
}

void FFS::InodeEntry::sterilize(std::ostream& stream) {
	stream.put(this->length);
	stream.put(this->tweet_blocks.size());
	for(unsigned long entry: tweet_blocks) {
		stream.put(entry);
	}
}

FFS::InodeEntry FFS::InodeEntry::desterilize(std::istream& stream) {
	int length, block_count; 
	stream >> length;
	stream >> block_count;

	std::vector<unsigned long> blocks;
	char get_ptr[8];
	while(block_count-- > 0) {
		stream.read(get_ptr, 8);
		unsigned long block_val = get_ptr[0] << 4 * 7;
		block_val |= get_ptr[1] << (4 * 6);
		block_val |= get_ptr[2] << (4 * 5);
		block_val |= get_ptr[3] << (4 * 4);
		block_val |= get_ptr[4] << (4 * 3);
		block_val |= get_ptr[5] << (4 * 2);
		block_val |= get_ptr[6] << (4 * 1);
		block_val |= get_ptr[7];

		blocks.push_back(block_val);
	}

	return *(new InodeEntry(length, blocks));
}

bool FFS::InodeEntry::operator==(const FFS::InodeEntry rhs) const {
	return this->length == rhs.length && this->tweet_blocks == rhs.tweet_blocks;
}

// Inode Table...

FFS::InodeTable::InodeTable(std::unordered_map<unsigned int, FFS::InodeEntry&>& entries) {
	this->entries = entries;
}

int FFS::InodeTable::size() {
	int size = 4; // 4 bytes for amount of entries
	for(auto entry: this->entries) {
		size += entry.second.size(); // Add the size for each entry
	}

	return size;
}

void FFS::InodeTable::sterilize(std::ostream& stream) {
	stream << this->entries.size();
	for(auto entry: this->entries) {
		int id = entry.first;
		stream << id;
		entry.second.sterilize(stream);
	}
}

FFS::InodeTable& FFS::InodeTable::desterilize(std::istream& stream) {
	int entries_count;
	stream >> entries_count;

	std::unordered_map<unsigned int, FFS::InodeEntry&> entries;
	while(entries_count-- > 0) {
		int id;
		stream >> id;
		InodeEntry entry = InodeEntry::desterilize(stream);
		entries.insert({id, entry});
	}

	return *(new InodeTable(entries));
}

void FFS::InodeTable::save(std::string path) {
	std::basic_fstream<char> stream;
	int size = this->size();

	this->sterilize(stream);
	create_image(path, stream, size);
}

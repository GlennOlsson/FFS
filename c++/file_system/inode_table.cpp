#include "inode_table.h"

#include <algorithm>
#include <filesystem>  //TODO: Remove
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "../helpers/functions.h"
#include "file_coder.h"
// Inode Entry...

FFS::InodeEntry::InodeEntry(int length,
							std::vector<unsigned long>* tweet_blocks) {
	this->length = length;
	this->tweet_blocks = tweet_blocks;
}

int FFS::InodeEntry::size() {
	int value = 0;
	value += 4;								  // Length field, int = 4 bytes
	value += 4;								  // Amount of entries = 4 bytes
	value += this->tweet_blocks->size() * 8;  // 8 bytes per element

	return value;
}

void FFS::InodeEntry::sterilize(std::ostream& stream) {
	// std::cout << "Entry... Length: " << this->length << ", nr blocks: " <<
	// this->tweet_blocks->size() << std::endl;
	write_i(stream, this->length);
	write_i(stream, this->tweet_blocks->size());
	// stream << this->length;
	// stream << this->tweet_blocks->size();
	for (unsigned long entry : *tweet_blocks) {
		write_l(stream, entry);
		std::cout << entry << std::endl;
	}
}

FFS::InodeEntry* FFS::InodeEntry::desterilize(std::istream& stream) {
	int length, block_count;

	read_i(stream, length);
	read_i(stream, block_count);

	// stream >> length;
	// stream >> block_count;

	std::vector<unsigned long>* blocks = new std::vector<unsigned long>();
	// char get_ptr[8];
	while (block_count-- > 0) {
		long signed_l;
		read_l(stream, signed_l);
		// stream.read(get_ptr, 8);
		// unsigned long block_val = get_ptr[0] << 4 * 7;
		// block_val |= get_ptr[1] << (4 * 6);
		// block_val |= get_ptr[2] << (4 * 5);
		// block_val |= get_ptr[3] << (4 * 4);
		// block_val |= get_ptr[4] << (4 * 3);
		// block_val |= get_ptr[5] << (4 * 2);
		// block_val |= get_ptr[6] << (4 * 1);
		// block_val |= get_ptr[7];

		unsigned long l = signed_l;

		blocks->push_back(l);
	}

	return new InodeEntry(length, blocks);
}

bool FFS::InodeEntry::operator==(const FFS::InodeEntry& rhs) const {
	std::cout << "compare entries... " << this->length << ", " << rhs.length
			  << std::endl;
	std::cout << "equalllll? "
			  << (this->length == rhs.length &&
						  (*this->tweet_blocks) == (*rhs.tweet_blocks)
					  ? "true"
					  : "false")
			  << std::endl;
	return this->length == rhs.length &&
		   (*this->tweet_blocks) == (*rhs.tweet_blocks);
}

// Inode Table...

FFS::InodeTable::InodeTable(std::map<unsigned int, FFS::InodeEntry*>* entries) {
	this->entries = entries;
}

int FFS::InodeTable::size() {
	int size = 4;  // 4 bytes for amount of entries
	for (auto entry : *this->entries) {
		size += 4;					   // Size of id, int
		size += entry.second->size();  // Add the size for each entry
	}

	return size;
}

void FFS::InodeTable::sterilize(std::ostream& stream) {
	std::cout << "Table Entries: " << this->entries->size() << std::endl;
	std::cout << "Should be 10..." << std::endl;
	// stream << this->entries->size();

	int total_entries = this->entries->size();
	// stream.put((total_entries >> (3 * 8)) & 0xFF);
	// stream.put((total_entries >> (2 * 8)) & 0xFF);
	// stream.put((total_entries >> (1 * 8)) & 0xFF);
	// stream.put((total_entries >> (0 * 8)) & 0xFF);

	write_i(stream, total_entries);

	std::cout << "output size bytes: " << std::endl;
	std::cout << "0x" << std::hex << ((total_entries >> (3 * 8)) & 0xFF)
			  << std::dec << std::endl;
	std::cout << "0x" << std::hex << ((total_entries >> (2 * 8)) & 0xFF)
			  << std::dec << std::endl;
	std::cout << "0x" << std::hex << ((total_entries >> (1 * 8)) & 0xFF)
			  << std::dec << std::endl;
	std::cout << "0x" << std::hex << ((total_entries >> (0 * 8)) & 0xFF)
			  << std::dec << std::endl;

	for (auto entry : *this->entries) {
		int id = entry.first;
		std::cout << "Entry inode: " << id << std::endl;
		write_i(stream, id);
		entry.second->sterilize(stream);
	}
}

FFS::InodeTable* FFS::InodeTable::desterilize(std::istream& stream) {
	int entries_count;

	// char a1;
	// char a2;
	// char a3;
	// char a4;

	// stream.get(a1);
	// stream.get(a2);
	// stream.get(a3);
	// stream.get(a4);

	// std::cout << "input size bytes: " << std::endl;
	// std::cout << "0x" << std::hex << a1 << std::dec << std::endl;
	// std::cout << "0x" << std::hex << a2 << std::dec << std::endl;
	// std::cout << "0x" << std::hex << a3 << std::dec << std::endl;
	// std::cout << "0x" << std::hex << a4 << std::dec << std::endl;

	// entries_count = (a1 << 3*8) | (a2 << 2*8) | (a3 << 1*8) | (a4 << 0*8);

	// stream >> entries_count;

	read_i(stream, entries_count);

	std::cout << "dester, count: " << entries_count << std::endl;

	std::map<unsigned int, FFS::InodeEntry*>* entries =
		new std::map<unsigned int, FFS::InodeEntry*>();
	while (entries_count-- > 0) {
		int signed_id;

		read_i(stream, signed_id);

		unsigned int id = signed_id;

		std::cout << "id: " << id << std::endl;

		// stream >> id;
		InodeEntry* entry = InodeEntry::desterilize(stream);
		entries->insert({id, entry});
	}

	return new InodeTable(entries);
}

void FFS::InodeTable::save(std::string path) {
	int size = this->size();

	std::basic_filebuf<char> buf;
	buf.open("/tmp/ffs_save",
	std::ios_base::in|std::ios_base::out|std::ios_base::binary|std::ios_base::trunc);

	std::basic_iostream stream(&buf);

	this->sterilize(stream);

	stream.seekg(0);

	create_image(path, stream, size);
}

FFS::InodeTable* FFS::InodeTable::load(std::string path) {
	// std::basic_fstream<char> stream("tmp",
	// std::ios_base::in|std::ios_base::out|std::ios_base::binary);

	std::basic_filebuf<char> buf;
	buf.open("/tmp/ffs_load",
	std::ios_base::in|std::ios_base::out|std::ios_base::binary|std::ios_base::trunc);

	std::basic_iostream stream(&buf);

	// std::ofstream ostream("tmp2", std::ofstream::binary);

	decode({path}, stream);

	// ostream.close();
	// std::filesystem::remove("tmp");

	//std::ifstream istream("tmp2", std::ofstream::binary);

	// std::cout << "is_open: " << (buf.is_open() ? "t" : "f") << std::endl;

	stream.seekg(0);

	auto table = desterilize(stream);

	// std::filesystem::remove("tmp");

	return table;
}

bool FFS::InodeTable::operator==(const FFS::InodeTable& rhs) const {

	//Compare size of tables (maps), and compare content of maps
	return this->entries->size() == rhs.entries->size() &&
		   std::equal(this->entries->begin(), this->entries->end(),
					  rhs.entries->begin(), [](auto e1, auto e2) {
						  // Compare keys, and de-reference InodeTable pointers
						  // and compare
						  return e1.first == e2.first &&
								 *e1.second == *e2.second;
					  });
}
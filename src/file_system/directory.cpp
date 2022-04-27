#include "directory.h"

#include "../helpers/functions.h"
#include "file_coder.h"

#include <string>
#include <sstream>

FFS::Directory::Directory(std::map<std::string, uint32_t>* entries) {
	this->entries = entries;
}

uint32_t FFS::Directory::size() {
	uint32_t size = 0;
	size += 4; // count_entries
	for(auto entry: *this->entries) {
		size += 1; // filename size
		size += entry.first.size(); // Length of filename
		size += 4; // inode id
	}

	return size;
}

void FFS::Directory::sterilize(std::ostream& stream) {
	FFS::write_i(stream, this->entries->size());
	for(auto entry: *this->entries) {
		std::string name = entry.first;

		uint8_t name_length = name.size();
		FFS::write_c(stream, name_length);

		// Write string without \0
		for(int i = 0; i < name_length; ++i) {
			FFS::write_c(stream, name[i]);
		}

		// Write inode id
		FFS::write_i(stream, entry.second);
	}
}
FFS::Directory* FFS::Directory::desterilize(std::istream& stream) {
	uint32_t entries_count;
	FFS::read_i(stream, entries_count);

	auto entries = new std::map<std::string, uint32_t>;

	while(entries_count--) {
		uint8_t name_count;
		FFS::read_c(stream, name_count);

		std::stringstream name_stream;
		while(name_count--) {
			uint8_t c;
			FFS::read_c(stream, c);
			name_stream.put(c);
		}

		std::string name = name_stream.str();

		uint32_t inode_id;

		// Write inode id
		FFS::read_i(stream, inode_id);

		entries->insert({name, inode_id});
	}

	return new FFS::Directory(entries);
}
std::vector<std::string> FFS::Directory::content() {
	std::vector<std::string> names(this->entries->size());
	for(auto entry: *this->entries)
		names.push_back(entry.first);
	
	return names;
}
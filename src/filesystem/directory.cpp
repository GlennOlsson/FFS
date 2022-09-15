#include "directory.h"

#include "storage.h"

#include "../helpers/functions.h"

#include "../system/state.h"

#include "file_coder.h"
#include "storage.h"

#include "../exceptions/exceptions.h"

#include <string>
#include <sstream>
#include <Magick++.h>
#include <memory>

FFS::Directory::Directory(std::shared_ptr<std::map<std::string, FFS::inode_t>> entries) {
	this->entries = entries;
}

FFS::Directory::Directory() {
	auto empty_entries = std::make_shared<std::map<std::string, FFS::inode_t>>();

	this->entries = empty_entries;
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

void FFS::Directory::serialize(std::ostream& stream) {
	FFS::write_c(stream, 'd');
	FFS::write_c(stream, 'i');
	FFS::write_c(stream, 'r');

	FFS::write_i(stream, this->entries->size());
	for(auto entry: *this->entries) {
		// Write inode id
		FFS::write_i(stream, entry.second);

		// Write name
		FFS::write_str(stream, entry.first);
	}
}
std::shared_ptr<FFS::Directory> FFS::Directory::deserialize(std::istream& stream) {
	// If stream is empty, just return empty Directory
	if(!stream || FFS::stream_size(stream) == 0)
		return std::make_shared<FFS::Directory>();

	char c1, c2, c3;
	FFS::read_c(stream, c1);
	FFS::read_c(stream, c2);
	FFS::read_c(stream, c3);
	if(c1 != 'd' || c2 != 'i' || c3 != 'r') {
		throw FFS::FFSFileNotDirectory();
	}

	uint32_t entries_count;
	FFS::read_i(stream, entries_count);

	auto entries = std::make_shared<std::map<std::string, uint32_t>>();

	while(entries_count-- > 0) {
		FFS::inode_t inode;
		// Read inode id
		FFS::read_i(stream, inode);

		std::string name;
		// Read string
		FFS::read_str(stream, name);
		entries->insert({name, inode});
	}

	return std::make_shared<FFS::Directory>(entries);
}

std::vector<std::string> FFS::Directory::content() {
	std::vector<std::string> names(this->entries->size());
	for(auto entry: *this->entries)
		names.push_back(entry.first);
	
	return names;
}

// Create file in directory
void FFS::Directory::add_entry(std::string name, FFS::inode_t id) {
	this->entries->insert({name, id});
}
// Get a file with specified name. Throws NoFileWithName exception if file does not exist
FFS::inode_t FFS::Directory::get_file(std::string name) {
	if(this->entries->contains(name))
		return this->entries->at(name);

	throw FFS::NoFileWithName(name);
}

FFS::inode_t FFS::Directory::remove_entry(std::string name) {
	if(this->entries->count(name) == 0)
		throw FFS::NoFileWithName(name);
		
	FFS::inode_t inode = this->entries->at(name);
	this->entries->erase(name);
	return inode;
}

bool FFS::Directory::operator==(const FFS::Directory& rhs) const {
	// Compare size of tables (maps), and compare content of maps
	return this->entries->size() == rhs.entries->size() &&
		   std::equal(this->entries->begin(), this->entries->end(),
					  rhs.entries->begin(), [](auto e1, auto e2) {
						  // Compare keys and values
						  return e1.first == e2.first &&
								 e1.second == e2.second;
					  });
}

#include "directory.h"

#include "../helpers/functions.h"
#include "file_coder.h"

#include <string>
#include <sstream>
#include <Magick++.h>

FFS::Directory::Directory(std::map<std::string, FFS::inode_id>* entries) {
	this->entries = entries;
}

FFS::Directory::Directory() {
	std::map<std::string, FFS::inode_id>* empty_entries = new std::map<std::string, FFS::inode_id>();

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

Magick::Blob* FFS::Directory::blob() {
	std::stringbuf buf;
	std::basic_iostream stream(&buf);

	this->sterilize(stream);

	uint32_t size = this->size();

	return create_image(stream, size);
}

FFS::Directory* FFS::Directory::from_blob(Magick::Blob* blob) {
	std::stringbuf buf;
	std::basic_iostream stream(&buf);

	std::vector<Magick::Blob*>* v = new std::vector<Magick::Blob*>();
	v->push_back(blob);
	decode(v, stream);

	return desterilize(stream);
}

std::vector<std::string> FFS::Directory::content() {
	std::vector<std::string> names(this->entries->size());
	for(auto entry: *this->entries)
		names.push_back(entry.first);
	
	return names;
}

// Create file in directory
void FFS::Directory::add_entry(std::string name, FFS::inode_id id) {
	this->entries->insert({name, id});
}
// Get a file with specified name. Throws NoFileWithName exception if file does not exist
FFS::inode_id FFS::Directory::get_file(std::string name) {
	return 0;
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

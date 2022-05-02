#include "catch.hpp"

// So we can access private fields of the class
#define private public

#include "../src/file_system/directory.h"
#include "../src/helpers/functions.h"
#include "../src/helpers/constants.h"
#include "../src/file_system/file_coder.h"

#include <sstream>
#include <iostream>
#include <fstream>


FFS::Directory* create_directory() {

	auto entries = new std::map<std::string, uint32_t>;

	// 10 files
	for(uint32_t i = 0; i < 10; i++) {
		uint32_t rand_inode_id = FFS::random_int();

		uint8_t rand_name_length = FFS::random_byte();
		std::stringstream name_stream;
		while(rand_name_length--) {
			uint8_t rand_c = FFS::random_byte();
			// Loop until we get an actual ASCII character
			while(rand_c <= 32 || rand_c >= 127)
				rand_c = FFS::random_byte();
			
			name_stream.put(rand_c);
		}

		std::string name = name_stream.str();

		entries->insert({name, rand_inode_id});
	}

	return new FFS::Directory(entries);
}

TEST_CASE("Sterlizing and desterlizing directory creates same dir", "[directory]") {
	FFS::Directory* directory = create_directory();

	std::string directory_output = "out.nosync/directory";

	directory->save(directory_output);
	FFS::Directory* desterilized_dir = FFS::Directory::load(directory_output);

	bool dirs_eq = *directory == *desterilized_dir;

	REQUIRE(dirs_eq);
}

TEST_CASE("Manipulated sterlizied data throws exception", "[directory]") {
	FFS::Directory* directory = create_directory();

	std::string directory_output = "out.nosync/directory";
	
	std::stringbuf buf;
	std::basic_iostream stream(&buf);

	directory->sterilize(stream);

	stream.seekp(100);
	stream.seekg(100);
	char c = stream.get();
	stream.put(~c);
	stream.flush();

	uint32_t size = directory->size();
	FFS::create_image(directory_output, stream, size);

	FFS::Directory* desterilized_dir = FFS::Directory::load(directory_output);

	bool dirs_eq = *directory == *desterilized_dir;

	REQUIRE(dirs_eq);
}
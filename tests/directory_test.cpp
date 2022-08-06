#include "catch.hpp"

// So we can access private fields of the class
#define private public

#include "../src/filesystem/directory.h"
#include "../src/filesystem/storage.h"
#include "../src/helpers/functions.h"
#include "../src/helpers/constants.h"
#include "../src/filesystem/file_coder.h"
#include "../src/exceptions/exceptions.h"

#include <sstream>
#include <iostream>
#include <fstream>
#include <exception>
#include <cassert>
#include <Magick++.h>

#define DIR_OUTPUT "out.nosync/directory.png"

std::shared_ptr<FFS::Directory> create_directory() {

	auto entries = std::make_shared<std::map<std::string, FFS::inode_t>>();

	// 10 files
	for(uint32_t i = 0; i < 10; i++) {
		FFS::inode_t rand_inode = FFS::random_int();

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

		entries->insert({name, rand_inode});
	}

	return std::make_shared<FFS::Directory>(entries);
}

TEST_CASE("Sterlizing and desterlizing directory creates same dir", "[directory]") {
	auto directory = create_directory();


	auto blobs = FFS::Storage::blobs(*directory);

	auto deserialized_dir = FFS::Storage::dir_from_blobs(blobs);

	bool dirs_eq = *directory == *deserialized_dir;

	REQUIRE(dirs_eq);
}

// Copies stream, flips byte at _at_ and creates FFS image of directory
// If it does not throw when creating image, check that directory and modified-directory are unequal
// If it does throw, make sure it is a FFS File exception
void test_flip_byte_and_create(uint32_t at, std::shared_ptr<FFS::Directory> dir) {
	uint32_t total_bytes = dir->size();
	assert(total_bytes > at);
	
	std::stringbuf buf;
	std::basic_iostream stream(&buf);
	dir->serialize(stream);

	std::stringstream cp_stream;

	uint32_t index = 0;
	char c;
	// Copy stream
	while(index++ < at) {
		c = stream.peek();
		cp_stream.put(c);
	}
	// Invert so we know it's not the same bytes
	c = stream.peek();
	cp_stream.put(~c);
	// Want to keep same size of buffer
	while(index++ < total_bytes) {
		c = stream.peek();
		cp_stream.put(c);
	}

	// Reset to original state
	stream.seekg(0);

	try { // If doesn't fail, make sure they are unequal
		auto blobs = FFS::encode(cp_stream);
		
		auto cp_dir = FFS::Storage::dir_from_blobs(blobs);
		bool dirs_eq = *cp_dir == *dir;

		REQUIRE_FALSE(dirs_eq);
	} catch(const FFS::BadFFSFile& b) {
		SUCCEED("Threw correct exception");
	} catch(std::exception& e) {
		std::stringstream ss;
		ss << "Threw wrong exception: " << e.what();
		FAIL(ss.str());
	}
}

TEST_CASE("Manipulated middle of sterlizied data leads to exception or unequal dir", "[directory]") {
	auto directory = create_directory();
	
	uint32_t total_bytes = directory->size();

	uint32_t middle = total_bytes / 2;

	test_flip_byte_and_create(middle, directory);
}

TEST_CASE("Manipulated start of sterlizied data leads to exception or unequal dir", "[directory]") {
	auto directory = create_directory();
	
	test_flip_byte_and_create(0, directory);
}

TEST_CASE("Manipulated end of sterlizied data leads to exception or unequal dir", "[directory]") {
	auto directory = create_directory();
	uint32_t total_bytes = directory->size();
	
	test_flip_byte_and_create(total_bytes - 1, directory);
}
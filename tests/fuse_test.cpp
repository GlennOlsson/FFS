#include "catch.hpp"

#include "helpers.h"

#include "../src/filesystem/fuse.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <iostream>
#include <thread>

#define FUSE_PATH "/tmp/ffs_fuse"

#define DIR "/foo"
#define FILE1 "/file.txt"
#define FILE2 "/foo/image.png"

#define TEXT_FILE "tests/assets.nosync/lorem.txt"
#define IMAGE_FILE "tests/assets.nosync/image.png"

bool fuse_is_running = false;

// Not sure which order tests are run in, but just want to start fuse once
void start_fuse() {
	if(fuse_is_running)
		return;
	std::cout << "Start fuse on " << FUSE_PATH << std::endl;
	std::cout << "Press enter when done...";
	
	std::cin >> i;
	int i = 0;

	std::cout << "OK, starting" << std::endl;

	fuse_is_running = true;

	// Remove current files
	auto content_it = std::filesystem::directory_iterator(FUSE_PATH);
	for(auto entry: content_it) {
		auto path = entry.path();
		std::filesystem::remove_all(path);
	}
}

void assert_file_is_same(std::filesystem::path real_path, std::filesystem::path ffs_path) {
    
	std::ifstream real_stream(real_path);
	std::ifstream ffs_stream(ffs_path);

    CHECK(streams_eq(real_stream, ffs_stream));
}

// ALL TESTS MUST REMOVE FILES AND DIRECTORIES THAT THEY CREATED

// TEST REQUIRES FUSE TO BE RUNNING AT DEFINED LOCATION

TEST_CASE("Test can create empty file and remove it", "[fuse]") {
	start_fuse();

	auto path = std::string(FUSE_PATH) + std::string(FILE1);
	auto fs_path = std::filesystem::path(path);

	// Make sure does not exist first
	CHECK(!std::filesystem::exists(fs_path));

	// Creates file	
	std::ofstream s(path);

	// Make sure exist after created
	CHECK(std::filesystem::exists(fs_path));
	// Make sure is 0 bytes, created as empty
	CHECK(std::filesystem::file_size(fs_path) == 0);

	std::filesystem::remove(fs_path);

	// Make sure does not exist after deleted
	CHECK(!std::filesystem::exists(fs_path));
}

TEST_CASE("Test can create empty dir and remove it", "[fuse]") {
	start_fuse();

	auto path = std::string(FUSE_PATH) + std::string(DIR);
	auto fs_path = std::filesystem::path(path);

	// Does not exist before
	CHECK(!std::filesystem::exists(fs_path));

	std::filesystem::create_directory(fs_path);
	
	// Exist after creation
	CHECK(std::filesystem::exists(fs_path));
	
	// Make sure dir is empty
	auto dir_it = std::filesystem::directory_iterator(fs_path);
	auto dir_elems = 0;
	for(auto it: dir_it) {
		dir_elems += 1;
	}
	CHECK(dir_elems == 0);

	std::filesystem::remove(fs_path);

	// Does not exist after deletion
	CHECK(!std::filesystem::exists(fs_path));
}

TEST_CASE("Test can create dir and empty file within it", "[fuse]") {
	start_fuse();

	auto dir_path = std::string(FUSE_PATH) + std::string(DIR);
	auto fs_dir_path = std::filesystem::path(dir_path);

	auto file_path = std::string(FUSE_PATH) + std::string(FILE2);
	auto fs_file_path = std::filesystem::path(file_path);

	// Does not exist before
	CHECK(!std::filesystem::exists(fs_file_path));

	// Create parent dir and empty file
	std::filesystem::create_directory(fs_dir_path);
	std::ofstream s(file_path);

	// Exist after creation
	CHECK(std::filesystem::exists(fs_file_path));
	
	// Make sure dir has exactly one entry
	auto dir_it = std::filesystem::directory_iterator(fs_dir_path);
	auto dir_elems = 0;
	for(auto it: dir_it) {
		dir_elems += 1;
	}
	CHECK(dir_elems == 1);

	// Remove file
	std::filesystem::remove(fs_file_path);

	// Make sure the dir is empty after removed
	dir_it = std::filesystem::directory_iterator(fs_dir_path);
	dir_elems = 0;
	for(auto it: dir_it) {
		dir_elems += 1;
	}
	CHECK(dir_elems == 0);

	// Remove dir as well
	std::filesystem::remove(fs_dir_path);

	// Does not exist after deletion
	CHECK(!std::filesystem::exists(fs_file_path));
	CHECK(!std::filesystem::exists(fs_dir_path));
}

// Covers; create, write and read
TEST_CASE("Can copy files, including into dir, and content is same", "[fuse]") {
	start_fuse();

	auto dir_path = std::string(FUSE_PATH) + std::string(DIR);
	auto fs_dir_path = std::filesystem::path(dir_path);

	auto file1_from_path = std::string(TEXT_FILE);
	auto fs_file1_from_path = std::filesystem::path(file1_from_path);

	auto file1_to_path = std::string(FUSE_PATH) + std::string(FILE1);
	auto fs_file1_to_path = std::filesystem::path(file1_to_path);

	auto file2_from_path = std::string(IMAGE_FILE);
	auto fs_file2_from_path = std::filesystem::path(file2_from_path);

	auto file2_to_path = std::string(FUSE_PATH) + std::string(FILE2);
	auto fs_file2_to_path = std::filesystem::path(file2_to_path);

	// Create parent dir
	std::filesystem::create_directory(fs_dir_path);

	// Copy files
	std::filesystem::copy_file(fs_file1_from_path, fs_file1_to_path);
	std::filesystem::copy_file(fs_file2_from_path, fs_file2_to_path);

	// Assert same files
	assert_file_is_same(file1_from_path, file1_to_path);
	assert_file_is_same(file2_from_path, file2_to_path);

	// Remove files and dir
	std::filesystem::remove(fs_file1_to_path);
	std::filesystem::remove(fs_file2_to_path);
	std::filesystem::remove(fs_dir_path);

	// Does not exist after deletion
	CHECK(!std::filesystem::exists(fs_file1_to_path));
	CHECK(!std::filesystem::exists(fs_file2_to_path));
	CHECK(!std::filesystem::exists(fs_dir_path));
}
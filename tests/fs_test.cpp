#include "catch.hpp"

#include "../src/filesystem/fs.h"
#include "../src/filesystem/directory.h"
#include "../src/filesystem/cache.h"

#include "../src/helpers/constants.h"

#include "../src/system/state.h"

#include "../src/exceptions/exceptions.h"

#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>

/* The filesystem for testing is defined as:
    /
        foo/
            fizz.txt
            buzz.pdf
            bar/
                qux.mov
        baz/

*/
#define TEST_PATH_ROOT FFS_TMP_FS_PATH

#define TEST_PATH_DIR_LEVEL_1 "/foo/"
#define TEST_PATH_TXT "/foo/fizz.txt"
#define TEST_PATH_PDF "/foo/buzz.pdf"
#define TEST_PATH_DIR_LEVEL_2 "/foo/bar/"
#define TEST_PATH_MOV "/foo/bar/qux.mov"
#define TEST_PATH_EMPTY_DIR "/baz/"

#define TEST_FILE_TXT "tests/assets.nosync/lorem.txt"
#define TEST_FILE_PDF "tests/assets.nosync/pdf.pdf"
#define TEST_FILE_MOV "tests/assets.nosync/movie.mp4"


// Removing inode table removes the reference to all files and dirs.
// Data is still saved on storage medium, but doesn't matter
void clear_fs() {
    FFS::State::clear_inode_table();

    // Clearing will remove it from the OWS, but in another thread. Could possibly find the old inode table first
    FFS::State::inode_table = std::make_shared<FFS::InodeTable>();

    FFS::Cache::clear_cache();
}

// Setup fs with files and directories created. Does not create files
void init_fs() {
    clear_fs();

    FFS::FS::create_dir(TEST_PATH_DIR_LEVEL_1);
    FFS::FS::create_dir(TEST_PATH_DIR_LEVEL_2);
    FFS::FS::create_dir(TEST_PATH_EMPTY_DIR);

    // Not necessary to upload to storage? Just keep in memory
    // FFS::State::save_table();
}

// Create files used in fs
void create_files() {
    // use one stream to save memory
    auto stream = std::make_shared<std::ifstream>(TEST_FILE_TXT);
    FFS::FS::create_file(TEST_PATH_TXT, stream);
    
    stream = std::make_shared<std::ifstream>(TEST_FILE_PDF);
    FFS::FS::create_file(TEST_PATH_PDF, stream);

    stream = std::make_shared<std::ifstream>(TEST_FILE_MOV);
    FFS::FS::create_file(TEST_PATH_MOV, stream);

    FFS::State::save_table();
}

bool streams_eq(std::basic_istream<char>& a, std::basic_istream<char>& b) {
	std::istreambuf_iterator<char> it1(a);
    std::istreambuf_iterator<char> it2(b);
	
	//Second argument is end-of-range iterator
	return std::equal(it1,std::istreambuf_iterator<char>(),it2); 
}

void assert_file_is_same(std::string real_path, std::string ffs_path) {
    std::ifstream input_stream(real_path);

    std::stringbuf buf;
    std::basic_iostream output_stream(&buf);

    FFS::FS::read_file(ffs_path, output_stream);
    // Reset stream to start. Both streams should now ouput the same
    REQUIRE(streams_eq(input_stream, output_stream));
}

TEST_CASE("Make sure created dirs exists and are empty", "[fs]") {
    // Dirs are empty as files are not created yet, done in test case
    init_fs();
    try {
        REQUIRE(FFS::FS::exists(TEST_PATH_DIR_LEVEL_1));
        auto dir_level_1 = FFS::FS::read_dir(TEST_PATH_DIR_LEVEL_1);
        REQUIRE(dir_level_1->entries->size() == 1); // contains the bar/ dir

        REQUIRE(FFS::FS::exists(TEST_PATH_DIR_LEVEL_2));
        auto dir_level_2 = FFS::FS::read_dir(TEST_PATH_DIR_LEVEL_2);
        REQUIRE(dir_level_2->entries->size() == 0);

        REQUIRE(FFS::FS::exists(TEST_PATH_EMPTY_DIR));
        auto dir_empty = FFS::FS::read_dir(TEST_PATH_EMPTY_DIR);
        REQUIRE(dir_empty->entries->size() == 0);
    } catch (FFS::NoFileWithName& e) {
        std::cout << e.what() << std::endl;
        FAIL("No file with name");
    }
}

TEST_CASE("Can create file and content is the same", "[fs]") {
    init_fs();
    create_files();

    REQUIRE(FFS::FS::exists(TEST_PATH_TXT));
    REQUIRE(FFS::FS::exists(TEST_PATH_PDF));
    REQUIRE(FFS::FS::exists(TEST_PATH_MOV));

    assert_file_is_same(TEST_FILE_TXT, TEST_PATH_TXT);
    assert_file_is_same(TEST_FILE_PDF, TEST_PATH_PDF);
    assert_file_is_same(TEST_FILE_MOV, TEST_PATH_MOV);
}

TEST_CASE("Reading dirs gives expected output", "[fs]") {
    init_fs();
    create_files();

    // Use one pointer for all dirs
    auto dir = FFS::FS::read_dir("/");
    auto content = dir->entries;
    REQUIRE(content->size() == 2);
    REQUIRE(content->count("foo") == 1);
    REQUIRE(content->count("baz") == 1);

    dir = FFS::FS::read_dir(TEST_PATH_DIR_LEVEL_1);
    content = dir->entries;
    REQUIRE(content->size() == 3);
    REQUIRE(content->count("bar") == 1);
    REQUIRE(content->count("fizz.txt") == 1);
    REQUIRE(content->count("buzz.pdf") == 1);

    dir = FFS::FS::read_dir(TEST_PATH_DIR_LEVEL_2);
    content = dir->entries;
    REQUIRE(content->size() == 1);
    REQUIRE(content->count("qux.mov") == 1);

    dir = FFS::FS::read_dir(TEST_PATH_EMPTY_DIR);
    content = dir->entries;
    REQUIRE(content->size() == 0);
}

TEST_CASE("Assert exists is false for non-existing files", "[fs]") {
    init_fs();
    // Only create dirs, not files

    REQUIRE_FALSE(FFS::FS::exists(TEST_PATH_TXT));
    REQUIRE_FALSE(FFS::FS::exists(TEST_PATH_PDF));
    REQUIRE_FALSE(FFS::FS::exists(TEST_PATH_MOV));
}

TEST_CASE("Can remove directory and created file, but nothing else is removed", "[fs]") {
    init_fs();
    create_files();

    // remove txt and level-2 dir (and all under it)
    FFS::FS::remove(TEST_PATH_TXT);
    FFS::FS::remove(TEST_PATH_DIR_LEVEL_2);

    // Assert what is removed is removed
    REQUIRE_FALSE(FFS::FS::exists(TEST_PATH_TXT));
    REQUIRE_FALSE(FFS::FS::exists(TEST_PATH_DIR_LEVEL_2));
    REQUIRE_FALSE(FFS::FS::exists(TEST_PATH_MOV));

    // Assert what is not removed is not removed
    REQUIRE(FFS::FS::exists(TEST_PATH_DIR_LEVEL_1));
    REQUIRE(FFS::FS::exists(TEST_PATH_PDF));
    REQUIRE(FFS::FS::exists(TEST_PATH_EMPTY_DIR));
}

TEST_CASE("Creating file or dir already existing throws", "[fs]") {
    init_fs();
    create_files();

    // Create existing file
    try {
        auto stream = std::make_shared<std::ifstream>(TEST_FILE_TXT);
        FFS::FS::create_file(TEST_PATH_TXT, stream);
        FAIL("Did not throw when creating file");
    } catch(FFS::FileAlreadyExists) {}

    // Create existing dir
    try {
        FFS::FS::create_dir(TEST_PATH_DIR_LEVEL_2);
        FAIL("Did not throw when creating dir");
    } catch(FFS::FileAlreadyExists) {}

    // Create dir on existing file location
    try {
        FFS::FS::create_dir(TEST_PATH_TXT);
        FAIL("Did not throw when creating dir on existing file location");
    } catch(FFS::FileAlreadyExists) {}

    // Create file on existing dir location
    try {
        auto stream = std::make_shared<std::ifstream>(TEST_FILE_TXT);
        FFS::FS::create_file(TEST_PATH_DIR_LEVEL_1, stream);
        FAIL("Did not throw when creating file on existing dir location");
    } catch(FFS::FileAlreadyExists) {}
}

TEST_CASE("Dirs and files are marked as such", "[fs]") {
    init_fs();
    create_files();

    REQUIRE(FFS::FS::is_dir(TEST_PATH_DIR_LEVEL_1));
    REQUIRE(FFS::FS::is_dir(TEST_PATH_DIR_LEVEL_2));
    REQUIRE(FFS::FS::is_dir(TEST_PATH_EMPTY_DIR));

    REQUIRE_FALSE(FFS::FS::is_dir(TEST_PATH_TXT));
    REQUIRE_FALSE(FFS::FS::is_dir(TEST_PATH_PDF));
    REQUIRE_FALSE(FFS::FS::is_dir(TEST_PATH_MOV));
}
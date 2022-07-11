#include "catch.hpp"

#include "../src/filesystem/fs.h"

#include "../src/helpers/constants.h"

#include "../src/system/state.h"

#include "../src/exceptions/exceptions.h"

#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>

/* The filesystem for testing is defined as:
    /
        foo/
            fizz.txt
            bar/
                buzz.pdf
                qux.mov
        baz/

*/
#define TEST_PATH_ROOT FFS_TMP_FS_PATH

#define TEST_PATH_DIR_LEVEL_1 "/foo/"
#define TEST_PATH_TXT "/foo/fizz.txt"
#define TEST_PATH_DIR_LEVEL_2 "/foo/bar/"
#define TEST_PATH_PDF "/foo/bar/buzz.pdf"
#define TEST_PATH_MOV "/foo/bar/qux.mov"
#define TEST_PATH_EMPTY_DIR "/baz/"

#define TEST_FILE_TXT "tests/assets.nosync/lorem.txt"
#define TEST_FILE_PDF "tests/assets.nosync/pdf.pdf"
#define TEST_FILE_MOV "tests/assets.nosync/movie.mp4"


// Remove all previous content in fs and re-create the root directory (empty dir)
void clear_fs() {
    uint removed_files = std::filesystem::remove_all(TEST_PATH_ROOT);
    std::cout << "Removed " << removed_files << " files" << std::endl;
    std::cout << (std::filesystem::create_directory(TEST_PATH_ROOT) ? "Created dir again" : "Could not create dir") << std::endl;
}

// Setup fs with files and directories created. Does not create files
void init_fs() {
    clear_fs();

    // Clear previous inode table from memory
    FFS::State::clear_inode_table();

    FFS::FS::create_dir(TEST_PATH_DIR_LEVEL_1);
    FFS::FS::create_dir(TEST_PATH_DIR_LEVEL_2);
    FFS::FS::create_dir(TEST_PATH_EMPTY_DIR);
}

// Create files used in fs
void create_files() {
    // use one stream to save memory
    std::ifstream* stream = new std::ifstream(TEST_FILE_TXT);
    FFS::FS::create_file(TEST_PATH_TXT, *stream);
    delete stream;
    
    stream = new std::ifstream(TEST_FILE_PDF);
    FFS::FS::create_file(TEST_PATH_PDF, *stream);
    delete stream;

    // stream = new std::ifstream(TEST_FILE_MOV);
    // FFS::FS::create_file(TEST_PATH_MOV, *stream);
    // delete stream;
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
    // REQUIRE(FFS::FS::exists(TEST_PATH_MOV));

    assert_file_is_same(TEST_FILE_TXT, TEST_PATH_TXT);
    assert_file_is_same(TEST_FILE_PDF, TEST_PATH_PDF);
    // assert_file_is_same(TEST_FILE_MOV, TEST_PATH_MOV);
}

TEST_CASE("Assert exists is false for non-existing files", "[fs]") {
    init_fs();
    // Only create dirs, not files

    REQUIRE_FALSE(FFS::FS::exists(TEST_PATH_TXT));
    init_fs();
    REQUIRE_FALSE(FFS::FS::exists(TEST_PATH_PDF));
    // REQUIRE_FALSE(FFS::FS::exists(TEST_PATH_MOV));
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
    REQUIRE_FALSE(FFS::FS::exists(TEST_PATH_PDF));
    // REQUIRE_FALSE(FFS::FS::exists(TEST_PATH_MOV));

    // Assert what is not removed is not removed
    REQUIRE(FFS::FS::exists(TEST_PATH_DIR_LEVEL_1));
    REQUIRE(FFS::FS::exists(TEST_PATH_TXT));
    REQUIRE(FFS::FS::exists(TEST_PATH_EMPTY_DIR));
}

TEST_CASE("Creating files already existing throws", "[fs]") {
    init_fs();
    create_files();

    // Create existing file
    try {
        std::ifstream stream(TEST_FILE_TXT);
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
        std::ifstream stream(TEST_FILE_TXT);
        FFS::FS::create_file(TEST_PATH_DIR_LEVEL_1, stream);
        FAIL("Did not throw when creating file on existing dir location");
    } catch(FFS::FileAlreadyExists) {}
}
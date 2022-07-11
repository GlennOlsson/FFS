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
        baz/

*/
#define TEST_PATH_ROOT FFS_TMP_FS_PATH

#define TEST_PATH_DIR_LEVEL_1 "/foo/"
#define TEST_PATH_TXT "/foo/fizz.txt"
#define TEST_PATH_DIR_LEVEL_2 "/foo/bar/"
#define TEST_PATH_PDF "/foo/bar/buzz.pdf"
#define TEST_PATH_EMPTY_DIR "/baz/"

#define TEST_FILE_TXT "tests/assets.nosync/lorem.txt"
#define TEST_FILE_PDF "tests/assets.nosync/pdf.pdf"


// Remove all previous content in fs and re-create the root directory (empty dir)
void clear_fs() {
    std::filesystem::remove_all(TEST_PATH_ROOT);
    std::filesystem::create_directory(TEST_PATH_ROOT);
}

// Setup fs with files and directories created. Don't create files yet, do that later
void init_fs() {
    clear_fs();

    // Clear previous inode table from memory
    FFS::State::clear_inode_table();

    FFS::FS::create_dir(TEST_PATH_DIR_LEVEL_1);
    FFS::FS::create_dir(TEST_PATH_DIR_LEVEL_2);
    FFS::FS::create_dir(TEST_PATH_EMPTY_DIR);
}

bool streams_eq(std::basic_istream<char>& a, std::basic_istream<char>& b) {
	std::istreambuf_iterator<char> it1(a);
    std::istreambuf_iterator<char> it2(b);
	
	//Second argument is end-of-range iterator
	return std::equal(it1,std::istreambuf_iterator<char>(),it2); 
}

TEST_CASE("Make sure created dirs exists and are empty", "[fs]") {
    // Dirs are empty as files are not created yet, done in test case
    init_fs();
    try {
        auto dir_level_1 = FFS::FS::read_dir(TEST_PATH_DIR_LEVEL_1);
        REQUIRE(dir_level_1->entries->size() == 1); // contains the bar/ dir

        auto dir_level_2 = FFS::FS::read_dir(TEST_PATH_DIR_LEVEL_2);
        REQUIRE(dir_level_2->entries->size() == 0);

        auto dir_empty = FFS::FS::read_dir(TEST_PATH_EMPTY_DIR);
        REQUIRE(dir_empty->entries->size() == 0);
    } catch (FFS::NoFileWithName& e) {
        std::cout << e.what() << std::endl;
        FAIL("No file with name");
    }
}

TEST_CASE("Can create file and content is the same", "[fs]") {
    init_fs();

    std::ifstream txt_input_stream(TEST_FILE_TXT);
    std::ifstream pdf_input_stream(TEST_FILE_PDF);

    FFS::FS::create_file(TEST_PATH_TXT, txt_input_stream);
    FFS::FS::create_file(TEST_PATH_PDF, pdf_input_stream);

    std::stringbuf buf;

    std::basic_iostream txt_output_stream(&buf);
    FFS::FS::read_file(TEST_PATH_TXT, txt_output_stream);
    // Reset stream to start. Both streams should now ouput the same
    txt_input_stream.seekg(0);
    REQUIRE(streams_eq(txt_input_stream, txt_output_stream));

    std::stringbuf buf2;
    std::basic_iostream pdf_output_stream(&buf2);
    FFS::FS::read_file(TEST_PATH_PDF, pdf_output_stream);
    pdf_input_stream.seekg(0);
    REQUIRE(streams_eq(pdf_input_stream, pdf_output_stream));
}
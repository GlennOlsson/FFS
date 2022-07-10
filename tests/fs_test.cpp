#include "catch.hpp"

#include "../src/filesystem/fs.h"

#include "../src/exceptions/exceptions.h"

#include <string>
#include <filesystem>
#include <iostream>

/* The filesystem for testing is defined as:
    /
        foo/
            fizz.txt
            bar/
                buzz.pdf
        baz/

*/
#define TEST_PATH_ROOT "/tmp/ffs_fs/"

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

    FFS::FS::create_dir(TEST_PATH_DIR_LEVEL_1);
    FFS::FS::create_dir(TEST_PATH_DIR_LEVEL_2);
    FFS::FS::create_dir(TEST_PATH_EMPTY_DIR);
}

TEST_CASE("Make sure created dirs exists and are empty", "[fs]") {
    // Dirs are empty as files are not created yet, done in test case
    init_fs();
    try {
        auto dir_level_1 = FFS::FS::read_dir(TEST_PATH_DIR_LEVEL_1);
        REQUIRE(dir_level_1->entries->size() == 0);

        auto dir_level_2 = FFS::FS::read_dir(TEST_PATH_DIR_LEVEL_2);
        REQUIRE(dir_level_2->entries->size() == 0);

        auto dir_empty = FFS::FS::read_dir(TEST_PATH_EMPTY_DIR);
        REQUIRE(dir_empty->entries->size() == 0);
    } catch (FFS::NoFileWithName& e) {
        std::cout << e.what() << std::endl;
        FAIL("No file with name");
    }
}


#include "catch.hpp"

#include <string>
#include <filesystem>

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

// Setup fs with files and directories created
void init_fs() {
    clear_fs();
   
    
}


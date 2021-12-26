
#pragma once

// Maximum possible possible number stored in 24 bits == header of output image
// 2^24 - 1
#define FFS_MAX_FILE_SIZE 16777215

// FFS Inode Table structure version
#define FFS_INODE_TABLE_VERSION 0

// FFS Output image file version
#define FFS_FILE_VERSION 0

// FFS Directory structure version
#define FFS_DIRECTORY_VERSION 0
namespace FFS {
	/**
	 * @brief Assert that all types used are of the expected byte length 
	 */
	void assert_correct_arch();
}
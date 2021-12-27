
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

// The image type outputted by Magick++
#define FFS_IMAGE_TYPE "png"
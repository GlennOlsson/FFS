#pragma once

// FFS image header length attribute is 32 bits so can store up to 4.2Gb of data
// But set max to 5 Mb
#define FFS_MAX_FILE_SIZE 5000000

// FFS Inode Table structure version
#define FFS_INODE_TABLE_VERSION 0

// FFS Output image file version
#define FFS_FILE_VERSION 0

// FFS Directory structure version
#define FFS_DIRECTORY_VERSION 0

// The image type outputted by Magick++
#define FFS_IMAGE_TYPE "png"

#define FFS_TMP_FS_PATH "out.nosync/tmp_fs"

// Inode ID for root directory
#define FFS_ROOT_INODE 0
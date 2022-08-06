#ifndef FFS_FILE_HANDLE_H
#define FFS_FILE_HANDLE_H

#include "../helpers/types.h"

#include <string>

namespace FFS::FileHandle {
	// Open a file or directory given a path, and return the file handle
	FFS::file_handle_t open(std::string);
	// Close file or directory with its file handle
	void close(FFS::file_handle_t);

	// Get the inode of given file handle
	FFS::inode_t inode(FFS::file_handle_t);
	// Get the parent inode of given file handle
	FFS::inode_t parent(FFS::file_handle_t);

	// Get the filename of given file handle
	std::string filename(FFS::file_handle_t);
}

#endif
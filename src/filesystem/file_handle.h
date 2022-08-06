#ifndef FFS_FILE_HANDLE_H
#define FFS_FILE_HANDLE_H

#include "../helpers/types.h"

#include <string>

typedef uint64_t file_handle_t;

namespace FFS::FileHandle {
	// Open a file or directory given a path, and return the file handle
	file_handle_t open(std::string);
	// Close file or directory with its file handle
	void close(file_handle_t);

	// Get the inode of given file handle
	FFS::inode_id inode(file_handle_t);
	// Get the parent inode of given file handle
	FFS::inode_id parent(file_handle_t);

	// Get the filename of given file handle
	std::string filename(file_handle_t);
}

#endif
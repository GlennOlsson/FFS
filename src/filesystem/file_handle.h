#ifndef FFS_FILE_HANDLE_H
#define FFS_FILE_HANDLE_H

#include "../helpers/types.h"

#include <string>
#include <memory>
#include <iostream>

namespace FFS::FileHandle {
	// Open a file or directory given a path, and return the file handle. 
	FFS::file_handle_t open(std::string);
	// Create a file given a path, and return file handle. Updates the parent dir in storage
	FFS::file_handle_t create(std::string);
	// Close file or directory with its file handle
	void close(FFS::file_handle_t);

	// Get the inode of given file handle
	FFS::inode_t inode(FFS::file_handle_t);
	// Get the parent inode of given file handle
	FFS::inode_t parent(FFS::file_handle_t);

	// Update stream and stream buffer of file handle which should be uploaded on close
	void update_stream(FFS::file_handle_t, std::shared_ptr<std::iostream>, std::shared_ptr<std::stringbuf>, bool);

	// Get current stream, not updated in storage	
	std::shared_ptr<std::iostream> get_stream(FFS::file_handle_t);

	// If the file or directory has been modified, i.e. if it has blobs to get
	bool is_cached(FFS::file_handle_t);
}

#endif
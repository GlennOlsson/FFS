#ifndef FFS_CACHE_H
#define FFS_CACHE_H

#include "directory.h"

#include "../helpers/types.h"

#include <memory>
#include <Magick++.h>
#include <istream>

// How many files and directories are cached by the system
#define FILE_CACHE_SIZE 10
#define DIRECTORY_CACHE_SIZE 10

// How many megabytes can a cached file or directory
#define MAX_CACHE_SIZE 5

// Biggest possible cache, in megabytes
#define CACHE_SIZE (MAX_CACHE_SIZE * (FILE_CACHE_SIZE + DIRECTORY_CACHE_SIZE))

namespace FFS::Cache {
	// File cache

	// Cache a file, and invalidate old cache for inode
	void cache(FFS::inode_id, std::shared_ptr<std::istream>);
	// Try to get the cache of a file. If not in cache, nullptr is returned
	std::shared_ptr<std::istream> get_file(FFS::inode_id);

	// Directory cache

	// Cache a directory, and invalidate old cache for inode
	void cache(FFS::inode_id, std::shared_ptr<FFS::Directory>);
	// Try to get the cache of a directory. If not in cache, nullptr is returned
	std::shared_ptr<FFS::Directory> get_dir(FFS::inode_id);
	// Cache the root dir
	void cache_root(std::shared_ptr<FFS::Directory>);
	// Get the root dir, or nullptr if not cached
	std::shared_ptr<FFS::Directory> get_root();
	// Invalidate root cache
	void invalidate_root();

	// Invalidate the cache of a file or directory
	void invalidate(FFS::inode_id);
}

#endif
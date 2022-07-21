#ifndef FFS_CACHE_H
#define FFS_CACHE_H

#include "directory.h"

#include "../helpers/types.h"

#include <memory>
#include <Magick++.h>
#include <istream>
#include <vector>

// How many files and directories are cached by the system
#define FILE_CACHE_SIZE 10
#define DIRECTORY_CACHE_SIZE 10

// How many megabytes can a cached file or directory
#define MAX_CACHE_SIZE 5

// Biggest possible cache, in megabytes
#define CACHE_SIZE (MAX_CACHE_SIZE * (FILE_CACHE_SIZE + DIRECTORY_CACHE_SIZE))

namespace FFS::Cache {
	// Cache the root dir
	void cache_root(std::shared_ptr<FFS::Directory>);
	// Get the root dir, or nullptr if not cached
	std::shared_ptr<FFS::Directory> get_root();
	// Invalidate root cache
	void invalidate_root();

	// Post cache
	void cache(FFS::post_id, std::shared_ptr<Magick::Blob>);
	std::shared_ptr<Magick::Blob> get(FFS::post_id);
	void invalidate(FFS::post_id);
}

#endif
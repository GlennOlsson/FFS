#pragma once
#include <string>
#include <Magick++.h>
#include <memory>
#include <vector>

namespace FFS {
	typedef uint32_t inode_t;
	// flickr id should be treated as string https://www.flickr.com/services/api/misc.overview.html
	typedef std::string post_id_t;

	typedef uint64_t file_handle_t;

	typedef std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> blobs_t;

	typedef std::shared_ptr<std::vector<FFS::post_id_t>> posts_t;
}
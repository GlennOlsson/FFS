#pragma once
#include <string>
namespace FFS {
	typedef uint32_t inode_t;
	// flickr id should be treated as string https://www.flickr.com/services/api/misc.overview.html
	typedef std::string post_id_t;

	typedef uint64_t file_handle_t;
}
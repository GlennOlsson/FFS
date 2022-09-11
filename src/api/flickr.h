#ifndef FFS_FLICKR_H
#define FFS_FLICKR_H

#include <string>

#include "../helpers/types.h"

namespace FFS::API::Flickr {

	struct SearchResponse {
		const std::string& url;
		const FFS::post_id_t& post_id;
	};

	// Post image to flickr, stored at file_path. post_text and tags are optional, but can be posted along with the photo
	FFS::post_id_t post_image(const std::string&, const std::string& tags = "");

	// Get the url to the image with a post_id_t. The url points to the original version of the image, as stored on flickr
	const std::string& get_image(const FFS::post_id_t&);

	// Get the most recent image uploaded to Flickr
	const SearchResponse most_recent_image();

	// Get the original url of the first image found with the tag, posted by the authenticated user. Will throw if no results are found
	const SearchResponse search_image(const std::string&);

	// Delete an image of flickr with specified post_id_t
	void delete_image(const FFS::post_id_t&);
}

#endif
#ifndef FFS_FLICKR_H
#define FFS_FLICKR_H

#include <string>

#include "../helpers/types.h"

namespace FFS::API::Flickr {
	// Post image to flickr, stored at file_path. post_text and tags are optional, but can be posted along with the photo
	FFS::post_id post_image(std::string file_path, std::string post_text = "", std::string tags = "");

	// Get the url to the image with a post_id. The url points to the original version of the image, as stored on flickr
	std::string get_image(FFS::post_id);

	// Get the post_id of the first image found with the tag, posted by the authenticated user. Will throw if no results are found
	FFS::post_id get_image(std::string);

	// Delete an image of flickr with specified post_id
	void delete_image(FFS::post_id);
}

#endif
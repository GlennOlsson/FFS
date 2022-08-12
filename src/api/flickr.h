#ifndef FFS_FLICKR_H
#define FFS_FLICKR_H

#include <string>

#include "../helpers/types.h"

namespace FFS::API::Flickr {
	// Post image to flickr, stored at file_path. post_text and tags are optional, but can be posted along with the photo
	FFS::post_id_t post_image(std::istream& stream, std::string tags = "");

	// Get the url to the image with a post_id_t. The url points to the original version of the image, as stored on flickr
	std::string get_image(FFS::post_id_t);

	// Get the original url of the first image found with the tag, posted by the authenticated user. Will throw if no results are found
	std::string search_image(std::string);

	// Delete an image of flickr with specified post_id_t
	void delete_image(FFS::post_id_t);
}

#endif
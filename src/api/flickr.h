#ifndef FFS_FLICKR_H
#define FFS_FLICKR_H

#include <string>
#include <cstdio>
#include "../helpers/types.h"

#define ENV_FFS_FLICKR_APP_CONSUMER_KEY "FFS_FLICKR_APP_CONSUMER_KEY"
#define ENV_FFS_FLICKR_APP_CONSUMER_SECRET "FFS_FLICKR_APP_CONSUMER_SECRET"
#define ENV_FFS_FLICKR_ACCESS_TOKEN "FFS_FLICKR_ACCESS_TOKEN"
#define ENV_FFS_FLICKR_ACCES_TOKEN_SECRET "FFS_FLICKR_ACCES_TOKEN_SECRET"

namespace FFS::API::Flickr {

	struct SearchResponse {
		const std::string& url;
		const FFS::post_id_t& post_id;
	};

	// Post image to flickr, stored at file_path of provided file size
	FFS::post_id_t post_image(const std::string&, const uint32_t);

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
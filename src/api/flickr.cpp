#include "flickr.h"

#include <flickcurl.h>
#include <string>
#include <iostream>
#include <exception>
#include <sstream>
#include <fstream>
#include <chrono>
#include <thread>

#include "../secret.h"
#include "curl.h"
#include "../exceptions/exceptions.h"
#include "../helpers/types.h"

#define FLICKR_USER_ID "me"
#define FLICKR_ORIGINAL_SIZE_STR "Original"

// Should change if the keys don't work
std::string oauth_key = FFS_FLICKR_ACCESS_TOKEN;
std::string oauth_secret = FFS_FLICKR_ACCES_TOKEN_SECRET;

void assert_keys_ok(flickcurl* fc) {
	// get user of keys
	auto user = std::string(flickcurl_test_login(fc));
	if(user != std::string(FFS_FLICKR_USER)) {
		throw FFS::BadFlickrKeys();
	}
}

void flickr_error_handler(void *user_data, const char *message) {
	auto error_str = std::string(message);
	throw FFS::FlickrException(error_str);
}

flickcurl* get_fc() {
	auto fc = flickcurl_new();
	
	flickcurl_set_error_handler(fc, flickr_error_handler, nullptr);

	flickcurl_set_oauth_client_key(fc, FFS_FLICKR_APP_CONSUMER_KEY);
  	flickcurl_set_oauth_client_secret(fc, FFS_FLICKR_APP_CONSUMER_SECRET);
	flickcurl_set_oauth_token(fc, oauth_key.c_str());
	flickcurl_set_oauth_token_secret(fc, oauth_secret.c_str());

	assert_keys_ok(fc);

	return fc;
}

FFS::post_id FFS::API::Flickr::post_image(std::string file_path, std::string post_text, std::string tags) {
	auto fc = get_fc();

	flickcurl_upload_params params;
	params.photo_file = file_path.c_str();
	params.title = post_text.c_str();
	params.tags = tags.c_str();
	params.is_public = true;
	auto status = flickcurl_photos_upload_params(fc, &params);
	auto uploaded_id = std::string(status->photoid);

	flickcurl_upload_status_free(status);

	flickcurl_free(fc);

	return uploaded_id;
}

std::string FFS::API::Flickr::get_image(FFS::post_id id) {
	auto fc = get_fc();

	auto sizes = flickcurl_photos_getSizes(fc, id.c_str());

	int index = 0;
	flickcurl_size* size_struct = sizes[index];
	while(strcmp(size_struct->label, FLICKR_ORIGINAL_SIZE_STR))
		size_struct = sizes[++index];

	auto src_str = std::string(size_struct->source);

	flickcurl_free_sizes(sizes);

	flickcurl_free(fc);

	return src_str;
}

FFS::post_id FFS::API::Flickr::search_image(std::string tag) {
	auto fc = get_fc();

	flickcurl_search_params search_params;
	flickcurl_search_params_init(&search_params);

	auto tag_len = tag.length();
	auto user_id_len = strlen(FLICKR_USER_ID);

	char* c_tag = new char[tag_len];
	char* c_user_id = new char[user_id_len];

	strcpy(c_tag, tag.c_str());
	strcpy(c_user_id, c_user_id);

	search_params.tags = c_tag;
	search_params.user_id = c_user_id;

	auto photos = flickcurl_photos_search(fc, &search_params);
	if(!photos)
		throw FFS::NoPhotoWithTag(tag);

	auto first_photo = *photos;
	auto id = std::string(first_photo->id);

	flickcurl_free_photos(photos);

	flickcurl_free(fc);

	return id;
}

void FFS::API::Flickr::delete_image(FFS::post_id id) {
	auto fc = get_fc();

 	flickcurl_photos_delete(fc, id.c_str());

	flickcurl_free(fc);
}
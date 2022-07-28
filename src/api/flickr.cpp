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

void destroy_fc(flickcurl* fc) {
	flickcurl_free(fc);
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

	destroy_fc(fc);

	return uploaded_id;
}

std::string FFS::API::Flickr::get_image(FFS::post_id) {

}

FFS::post_id FFS::API::Flickr::search_image(std::string) {

}

void FFS::API::Flickr::delete_image(FFS::post_id) {

}


void flickr() {
	


	// auto sizes = flickcurl_photos_getSizes(fc, "52242386938");

	// int index = 0;
	// flickcurl_size* size_struct = sizes[index];
	// while(std::string(size_struct->label) != std::string("Original")) {
	// 	size_struct = sizes[++index];
	// }

	// std::cout << "URL: " << size_struct->source << std::endl;

	// auto image_stream = FFS::API::HTTP::get(std::string(size_struct->source));
	// std::ofstream out_stream("flickr_img.png", std::ios::binary);

	// out_stream << image_stream->rdbuf();

	// char* tag = new char[10];
 	// char* user_id = new char[3];

	// strcpy(tag, std::string("ffs_inode").c_str());
	// strcpy(user_id, std::string("me").c_str());

	// std::cout << "UserID: " << user_id << ", tag: " << tag << std::endl;

	// flickcurl_upload_params params;
	// params.photo_file = "/Users/glenn/Documents/KTH/CalPoly/Thesis/src/out.nosync/tmp_fs/ffs_0.png";
	// params.title = "Hejhopp";
	// params.tags = tag;
	// params.is_public = true;

	// auto status = flickcurl_photos_upload_params(fc, &params);
	// auto uploaded_id = status->photoid;

	// std::cout << "ID: " << uploaded_id << std::endl;

	// std::this_thread::sleep_for(std::chrono::seconds(10));

	// flickcurl_search_params search_params;
	// flickcurl_search_params_init(&search_params);

	// search_params.tags = tag;
	// search_params.user_id = user_id;

	// // Assume first photo is the photo
	// auto photos = flickcurl_photos_search(fc, &search_params);
	// if(!photos) {
	// 	std::cout << "photos is not a good address " << std::endl;
	// 	return;
	// }

	// std::cout << "add: " << photos << std::endl;

	// auto inode_table = *photos;

	// auto indode_table_id = inode_table->id;
	// std::cout << "Found inode table id: " << indode_table_id << std::endl;

	// auto id_to_delete = "52247246015";
	// auto del = flickcurl_photos_delete(fc, id_to_delete);
	// std::cout << "del: " << del << std::endl;

	
}
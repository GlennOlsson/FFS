#include "flickr.h"

#include <flickcurl.h>
#include <string>
#include <iostream>
#include <exception>
#include <sstream>
#include <fstream>
#include <chrono>
#include <thread>
#include <liboauthcpp/liboauthcpp.h>
#include <stdlib.h>

#include "curl.h"
#include "../exceptions/exceptions.h"
#include "../helpers/types.h"
#include "../helpers/logger.h"
#include "../helpers/functions.h"
#include "json.h"

#define FLICKR_USER_ID "me"
#define FLICKR_ORIGINAL_SIZE_STR "Original"

#define BASE_REST_URL std::string("https://api.flickr.com/services/rest")
#define BASE_UPLOAD_URL "https://api.flickr.com/services/upload/"

#define BASE_REST_PARAMS std::string("format=json&nojsoncallback=1")

#define RETRIES 3

// Should change if the keys don't work
std::string access_key = getenv(ENV_FFS_FLICKR_ACCESS_TOKEN);
std::string access_secret =  getenv(ENV_FFS_FLICKR_ACCES_TOKEN_SECRET);
std::string consumer_key = getenv(ENV_FFS_FLICKR_APP_CONSUMER_KEY);
std::string consumer_secret = getenv(ENV_FFS_FLICKR_APP_CONSUMER_SECRET);

int request_counter = 0;

std::string get_auth_string(OAuth::Http::RequestType request_type, std::string full_url) {
	OAuth::Consumer consumer(consumer_key, consumer_secret);
    OAuth::Token token(access_key, access_secret);
    OAuth::Client oauth(&consumer, &token);

	std::string str = oauth.getURLQueryString(request_type, full_url);
	
	return str;
}

class UploadFailed {};

void flickr_error_handler(void *user_data, const char *message) {
	auto error_str = std::string(message);
	FFS::err << "Upload failed: " << error_str << std::endl;
	throw UploadFailed();
}

flickcurl* get_fc() {
	auto fc = flickcurl_new();

	flickcurl_set_error_handler(fc, flickr_error_handler, nullptr);

	flickcurl_set_oauth_client_key(fc, consumer_key.c_str());
  	flickcurl_set_oauth_client_secret(fc, consumer_secret.c_str());
	flickcurl_set_oauth_token(fc, access_key.c_str());
	flickcurl_set_oauth_token_secret(fc, access_secret.c_str());

	return fc;
}

FFS::post_id_t FFS::API::Flickr::post_image(const std::string& file_path) {

	auto fc = get_fc();

	// Generate some random string of 7-15 characters as the title
	auto title = FFS::random_str(FFS::random_int(7, 15));

	flickcurl_upload_params params;

	params.photo_file = file_path.c_str();
	params.title = title.c_str();
	params.description = nullptr;
	params.tags = nullptr;
	params.is_public = true;
	params.is_friend = 0;
	params.is_family = 0;
	params.safety_level = 0;
	params.content_type = 0;
	params.hidden = 0;

	FFS::log << "Uploading file, request counter: " << request_counter << std::endl;

	flickcurl_upload_status* status = nullptr;
	for(int i = 0; i < RETRIES; ++i) {
		try {
			status = flickcurl_photos_upload_params(fc, &params);
			break;
		} catch(const UploadFailed& e) {
			if(i == RETRIES - 1) {
				FFS::err << "Retried " << RETRIES << " times, still failed" << std::endl;
				throw FFS::FlickrException("Could not upload file after " + std::to_string(RETRIES) + " attempts");
			}
		}
	}

	auto uploaded_id = std::string(status->photoid);

	request_counter++;

	flickcurl_free_upload_status(status);

	flickcurl_free(fc);

	return uploaded_id;
}

const std::string& _get_image(const FFS::post_id_t& id) {
	std::string method = "flickr.photos.getSizes";

	auto full_params = BASE_REST_PARAMS + "&method=" + method + "&photo_id=" + id;

	auto auth_str = get_auth_string(OAuth::Http::Get, BASE_REST_URL + "?" + full_params);

	auto response_body = FFS::API::HTTP::get(BASE_REST_URL, auth_str);

	request_counter++;
	FFS::log << "Getting file sizes, request counter: " << request_counter << std::endl;

	auto json = FFS::API::JSON::parse(*response_body);

	try {
		auto& sizes_obj = json->get_obj("sizes");
		auto& size_arr = sizes_obj.get_arr("size");

		for(auto elem: size_arr) {
			auto& size_obj = FFS::API::JSON::as_obj(elem);
			auto label = size_obj.get_str("label");
			if(label == FLICKR_ORIGINAL_SIZE_STR) {
				auto& url = size_obj.get_str("source");
				return url;
			}
		}
	} catch(FFS::JSONKeyNonexistant& e) {
		throw FFS::NoPhotoWithID(id);
	}
	// if not found
	throw FFS::NoPhotoWithID(id);
}

const std::string& FFS::API::Flickr::get_image(const FFS::post_id_t& id) {
	for(int i = 0; i < RETRIES; ++i) {
		try {
			return _get_image(id);
		} catch(const std::exception& e) {
			if(i == RETRIES - 1) {
				FFS::err << "Retried " << RETRIES << " times, still failed" << std::endl;
				throw e;
			}
		}
	}
	FFS::err << "Somehow didn't return or throw on get_image" << std::endl;
	throw FFS::FlickrException("Error with get_image");
}

const FFS::API::Flickr::SearchResponse _most_recent_image() {
	std::string method = "flickr.people.getPhotos";

	auto full_params = BASE_REST_PARAMS + 
		"&method=" + method + 
		"&user_id=" + FLICKR_USER_ID + 
		"&extras=url_o" + 
		"&per_page=1";

	auto auth_str = get_auth_string(OAuth::Http::Get, BASE_REST_URL + "?" + full_params);

	FFS::log << "Getting most recent image info, request counter: " << request_counter << std::endl;
	
	auto response_body = FFS::API::HTTP::get(BASE_REST_URL, auth_str);

	request_counter++;

	auto json = FFS::API::JSON::parse(*response_body);

	try {
		auto& photos_obj = json->get_obj("photos");

		auto& photo_arr = photos_obj.get_arr("photo");
		if(photo_arr.size() < 1) {
			FFS::log << "No photos in array" << std::endl;
			throw FFS::NoPhotosOnFlickr();
		}

		auto& first_photo_obj = FFS::API::JSON::as_obj(photo_arr.front());
		auto& o_url = first_photo_obj.get_str("url_o");
		auto& id = first_photo_obj.get_str("id");

		FFS::API::Flickr::SearchResponse sr = {o_url, id};

		return sr;
	} catch(FFS::JSONKeyNonexistant& e) {
		FFS::log << "JSON key error with most recent flickr image" << std::endl;
		throw FFS::NoPhotosOnFlickr();
	}
}

const FFS::API::Flickr::SearchResponse FFS::API::Flickr::most_recent_image() {
	for(int i = 0; i < RETRIES; ++i) {
		try {
			return _most_recent_image();
		} catch(const std::exception& e) {
			if(i == RETRIES - 1) {
				FFS::err << "Retried " << RETRIES << " times, still failed" << std::endl;
				throw e;
			}
		}
	}
	FFS::err << "Somehow didn't return or throw on most_recent_image" << std::endl;
	throw FFS::FlickrException("Error with most_recent_image");
}

const FFS::API::Flickr::SearchResponse _search_image(const std::string& tag) {
	std::string method = "flickr.photos.search";

	auto full_params = BASE_REST_PARAMS + 
		"&method=" + method + 
		"&user_id=" + FLICKR_USER_ID + 
		"&tags=" + tag +
		"&tag_mode=all&extras=url_o&per_page=1";

	auto auth_str = get_auth_string(OAuth::Http::Get, BASE_REST_URL + "?" + full_params);
	
	FFS::log << "Searching for photo, request counter: " << request_counter << std::endl;

	auto response_body = FFS::API::HTTP::get(BASE_REST_URL, auth_str);

	request_counter++;

	auto json = FFS::API::JSON::parse(*response_body);

	try {
		auto& photos_obj = json->get_obj("photos");
		auto& photo_arr = photos_obj.get_arr("photo");
		if(photo_arr.size() < 1)
			throw FFS::NoPhotoWithTag(tag);

		auto& first_photo_obj = FFS::API::JSON::as_obj(photo_arr.front());
		auto& o_url = first_photo_obj.get_str("url_o");
		auto& id = first_photo_obj.get_str("id");

		FFS::API::Flickr::SearchResponse sr = {o_url, id};

		return sr;
	} catch(FFS::JSONKeyNonexistant& e) {
		throw FFS::NoPhotoWithTag(tag);
	}
}

const FFS::API::Flickr::SearchResponse FFS::API::Flickr::search_image(const std::string& tag) {
	for(int i = 0; i < RETRIES; ++i) {
		try {
			return _search_image(tag);
		} catch(const std::exception& e) {
			if(i == RETRIES - 1) {
				FFS::err << "Retried " << RETRIES << " times, still failed" << std::endl;
				throw e;
			}
		}
	}
	FFS::err << "Somehow didn't return or throw on search_image" << std::endl;
	throw FFS::FlickrException("Error with search_image");
}

void _delete_image(const FFS::post_id_t& id) {
	std::string method = "flickr.photos.delete";

	auto full_params = BASE_REST_PARAMS + 
		"&method=" + method + 
		"&photo_id=" + id;

	FFS::log << "Deleting file, request counter: " << request_counter << std::endl;
	
	auto auth_str = get_auth_string(OAuth::Http::Get, BASE_REST_URL + "?" + full_params);

	// For some reason problem if using post instead of get - but get works fine!!
	FFS::API::HTTP::get(BASE_REST_URL, auth_str);

	request_counter++;
}

void FFS::API::Flickr::delete_image(const FFS::post_id_t& id) {
	for(int i = 0; i < RETRIES; ++i) {
		try {
			return _delete_image(id);
		} catch(const std::exception& e) {
			if(i == RETRIES - 1) {
				FFS::err << "Retried " << RETRIES << " times, still failed" << std::endl;
				throw e;
			}
		}
	}
	FFS::err << "Somehow didn't return or throw on delete_image" << std::endl;
	throw FFS::FlickrException("Error with delete_image");
}
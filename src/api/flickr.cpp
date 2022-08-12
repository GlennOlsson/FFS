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

#include "../secret.h"
#include "curl.h"
#include "../exceptions/exceptions.h"
#include "../helpers/types.h"
#include "../helpers/logger.h"

#define FLICKR_USER_ID "me"
#define FLICKR_ORIGINAL_SIZE_STR "Original"

// Should change if the keys don't work
std::string oauth_key = FFS_FLICKR_ACCESS_TOKEN;
std::string oauth_secret = FFS_FLICKR_ACCES_TOKEN_SECRET;

std::string get_auth_string(OAuth::Http::RequestType request_type, std::string full_url) {
	OAuth::Consumer consumer(FFS_FLICKR_APP_CONSUMER_KEY, FFS_FLICKR_APP_CONSUMER_SECRET);
    OAuth::Token token(FFS_FLICKR_ACCESS_TOKEN, FFS_FLICKR_ACCES_TOKEN_SECRET);
    OAuth::Client oauth(&consumer, &token);

	std::string str = oauth.getURLQueryString(request_type, full_url);
	
	return str;
}

FFS::post_id_t FFS::API::Flickr::post_image(std::istream& stream, std::string tags) {

	return "";
}

std::string FFS::API::Flickr::get_image(FFS::post_id_t id) {
	
	return "";
}

std::string FFS::API::Flickr::search_image(std::string tag) {

	return "";
}

void FFS::API::Flickr::delete_image(FFS::post_id_t id) {

}
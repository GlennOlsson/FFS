#include "../config.h"

#ifdef USE_LOCAL_STORAGE
#include "local.h"

#include <filesystem>
#include <string>
#include <memory>
#include <fstream>

#include "../helpers/types.h"
#include "../helpers/functions.h"

#include "../exceptions/exceptions.h"

#define INODE_POST_ID std::string("i")

std::string path_of(const FFS::post_id_t& id) {
	return "/tmp/ffs/" + id;
}

FFS::post_id_t FFS::API::Local::save_file(const std::string& from_path, bool is_inode_table) {
	auto id = is_inode_table ? INODE_POST_ID : std::to_string(FFS::random_int());
	auto save_path = path_of(id);
	
	std::filesystem::copy_file(from_path, save_path, std::filesystem::copy_options::overwrite_existing);

	return id;
}

std::shared_ptr<std::istream> FFS::API::Local::get_file(const FFS::post_id_t& post) {
	auto path = path_of(post);
	if(!std::filesystem::exists(path))
		throw FFS::NoPhotoWithID(post);
	return std::make_shared<std::ifstream>(path);
}

FFS::post_id_t FFS::API::Local::get_inode_post_id() {
	return INODE_POST_ID;
}

void FFS::API::Local::delete_file(const FFS::post_id_t& post) {
	auto path = path_of(post);
	std::filesystem::remove(path);
}

#endif
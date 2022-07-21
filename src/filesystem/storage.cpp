#include "storage.h"

#include "file_coder.h"
#include "cache.h"

#include "../helpers/types.h"
#include "../helpers/functions.h"
#include "../helpers/constants.h"

#include "../system/state.h"

#include <Magick++.h>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <thread>

std::string path_of(FFS::post_id id) {
	std::stringstream path_stream;
	path_stream << FFS_TMP_FS_PATH << "/ffs_" << std::to_string(id) << "." << FFS_IMAGE_TYPE;
	return path_stream.str();
}


std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> FFS::Storage::blobs(FFS::Directory& dir) {
	std::stringbuf buf;
	std::basic_iostream stream(&buf);

	dir.serialize(stream);

	return FFS::encode(stream);
}

std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> FFS::Storage::blobs(FFS::InodeTable& table) {
	std::stringbuf buf;
	std::basic_iostream stream(&buf);

	table.serialize(stream);

	return FFS::encode(stream);
}

std::shared_ptr<FFS::Directory> FFS::Storage::dir_from_blobs(std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> blobs) {
	std::stringbuf buf;
	std::basic_iostream stream(&buf);

	FFS::decode(blobs, stream);

	return FFS::Directory::deserialize(stream);
}

std::shared_ptr<FFS::InodeTable> FFS::Storage::itable_from_blobs(std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> blobs) {
	std::stringbuf buf;
	std::basic_iostream stream(&buf);

	FFS::decode(blobs, stream);

	return FFS::InodeTable::deserialize(stream);
}

FFS::inode_id FFS::Storage::upload(std::shared_ptr<Directory> dir) {
	auto inode = FFS::Storage::upload_and_save_file(FFS::Storage::blobs(*dir), dir->size(), true);

	return inode;
}

void FFS::Storage::update(std::shared_ptr<FFS::Directory> dir, FFS::inode_id inode_id) {
	FFS::Cache::invalidate(inode_id);

	auto new_post_ids = FFS::Storage::upload_file(FFS::Storage::blobs(*dir));
	auto table = FFS::State::get_inode_table();
	auto inode_entry = table->entry(inode_id);

	// remove old dir from storage device
	FFS::Storage::remove_blocks(*inode_entry->post_blocks);
	inode_entry->post_blocks = new_post_ids;

	FFS::Cache::cache(inode_id, dir);
	FFS::State::save_table();
}

void FFS::Storage::save_file(FFS::post_id id, std::shared_ptr<Magick::Blob> blob) { 
	std::string path = path_of(id);

	Magick::Image img(*blob);
	img.write(path);
}

FFS::post_id _upload_file(std::shared_ptr<Magick::Blob> blob) {
	// Assume no collision as it's 64-bit, i.e. 1.8e19 choices
	FFS::post_id id = FFS::random_long();
	FFS::Storage::save_file(id, blob);
	return id;
}

FFS::inode_id FFS::Storage::upload_and_save_file(std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> blobs, size_t size, bool is_dir) {
	auto posts = std::make_shared<std::vector<FFS::post_id>>();
	for(auto blob: *blobs) {
		FFS::post_id id = _upload_file(blob);
		posts->push_back(id);
	}

	auto table = FFS::State::get_inode_table();

	return table->new_file(posts, size, is_dir);
}


std::shared_ptr<std::vector<FFS::post_id>> FFS::Storage::upload_file(std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> blobs) {
	auto posts = std::make_shared<std::vector<FFS::post_id>>();

	for(auto blob: *blobs) {
		FFS::post_id id = _upload_file(blob);
		posts->push_back(id);
	}

	return posts;
}

std::shared_ptr<Magick::Blob> FFS::Storage::get_file(FFS::post_id id) {
	std::string path = path_of(id);
	Magick::Image img(path);

	auto blob = std::make_shared<Magick::Blob>();
	img.write(blob.get());

	// Simulate time to fetch from online service
	std::this_thread::sleep_for(std::chrono::milliseconds(500));

	return blob;
}

std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> FFS::Storage::get_file(std::shared_ptr<std::vector<FFS::post_id>> ids) {
	// TODO: Cache and check cache?
	auto v = std::make_shared<std::vector<std::shared_ptr<Magick::Blob>>>();
	for(auto id: *ids) {
		v->push_back(get_file(id));
	}
	return v;
}

void FFS::Storage::remove_blocks(std::vector<FFS::post_id>& blocks) {
	for(auto block: blocks) {
		auto path = path_of(block);
		std::filesystem::remove(path);
	}
}
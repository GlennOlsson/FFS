#include "storage.h"

#include "file_coder.h"
#include "cache.h"

#include "../helpers/types.h"
#include "../helpers/functions.h"
#include "../helpers/constants.h"

#include "../system/state.h"

#include "../api/flickr.h"
#include "../api/curl.h"

#include "../exceptions/exceptions.h"

#include <Magick++.h>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <thread>

#define FFS_INODE_TABLE_TAG "ffs_inode"

std::string path_of(FFS::post_id id) {
	std::stringstream path_stream;
	path_stream << FFS_TMP_FS_PATH << "/ffs_" << id << "." << FFS_IMAGE_TYPE;
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

std::shared_ptr<FFS::InodeTable> FFS::Storage::itable_from_blob(std::shared_ptr<Magick::Blob> blob) {
	std::stringbuf buf;
	std::basic_iostream stream(&buf);

	auto blobs = std::make_shared<std::vector<std::shared_ptr<Magick::Blob>>>();
	blobs->push_back(blob);

	FFS::decode(blobs, stream);

	return FFS::InodeTable::deserialize(stream);
}

FFS::inode_id FFS::Storage::upload(std::shared_ptr<Directory> dir) {
	return FFS::Storage::upload_and_save_file(FFS::Storage::blobs(*dir), dir->size(), true);;
}

void FFS::Storage::update(std::shared_ptr<FFS::Directory> dir, FFS::inode_id inode_id) {
	auto new_post_ids = FFS::Storage::upload_file(FFS::Storage::blobs(*dir));
	auto table = FFS::State::get_inode_table();
	auto inode_entry = table->entry(inode_id);

	// remove old dir from storage device
	FFS::Storage::remove_posts(*inode_entry->post_blocks);
	inode_entry->post_blocks = new_post_ids;
}

FFS::post_id FFS::Storage::upload_file(std::shared_ptr<Magick::Blob> blob, bool is_inode) {
	// Write to temporary file, upload, and then remove temp file
	auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	std::cout << "Uploading " << (is_inode ? "Inode table" : "file/dir") << std::endl;

	auto tmp_filename = "/tmp/ffs_" + std::to_string(FFS::random_int());

	Magick::Image img(*blob);
	img.write(tmp_filename);

	std::string tag = "";
	if(is_inode) {
		tag = FFS_INODE_TABLE_TAG;
	}

	auto id = FFS::API::Flickr::post_image(tmp_filename, "", tag);

	std::filesystem::remove(tmp_filename);

	FFS::Cache::cache(id, blob);

	auto done_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch());
	std::cout << "Took: " << (done_time.count() - now.count()) << std::endl << std::endl;
	
	return id;
}

std::shared_ptr<std::vector<FFS::post_id>> FFS::Storage::upload_file(std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> blobs) {
	auto posts = std::make_shared<std::vector<FFS::post_id>>();

	for(auto blob: *blobs) {
		FFS::post_id id = upload_file(blob);
		posts->push_back(id);
	}

	return posts;
}

FFS::inode_id FFS::Storage::upload_and_save_file(std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> blobs, size_t size, bool is_dir) {
	auto posts = upload_file(blobs);

	auto table = FFS::State::get_inode_table();

	return table->new_file(posts, size, is_dir);
}

std::shared_ptr<Magick::Blob> FFS::Storage::get_file(FFS::post_id id) {
	if(FFS::Cache::get(id) != nullptr)
		return FFS::Cache::get(id);

	auto source_url = FFS::API::Flickr::get_image(id);
	auto file_stream = FFS::API::HTTP::get(source_url);

	// Read stream length
	file_stream->seekg(0, file_stream->end);
	auto length = file_stream->tellg();
	file_stream->seekg(0, file_stream->beg);

	// Copy stream to data array for Blob
	int index = 0;
	char* data = new char[length];
	while(*file_stream) {
		FFS::read_c(*file_stream, data[index++]);
	}

	auto blob = std::make_shared<Magick::Blob>(data, length);

	delete[] data;

	FFS::Cache::cache(id, blob);

	return blob;
}

std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> FFS::Storage::get_file(std::shared_ptr<std::vector<FFS::post_id>> ids) {
	auto v = std::make_shared<std::vector<std::shared_ptr<Magick::Blob>>>();
	for(auto id: *ids) {
		v->push_back(get_file(id));
	}
	return v;
}

FFS::post_id FFS::Storage::get_inode_table() {
	std::string tag = FFS_INODE_TABLE_TAG;

	auto post_id = FFS::API::Flickr::search_image(tag);
	return post_id;
}

void FFS::Storage::remove_post(FFS::post_id& post_id) {
	FFS::API::Flickr::delete_image(post_id);

	FFS::Cache::invalidate(post_id);
}

void FFS::Storage::remove_posts(std::vector<FFS::post_id>& posts) {
	for(auto post_id: posts) {
		remove_post(post_id);
	}
}
#include "../config.h"

#include "storage.h"

#include "file_coder.h"
#include "cache.h"

#include "../helpers/types.h"
#include "../helpers/functions.h"
#include "../helpers/constants.h"

#include "../system/state.h"

#include "../api/flickr.h"
#include "../api/curl.h"

#ifdef DEBUG
#include "../api/local.h"
#endif

#include "../exceptions/exceptions.h"

#include <Magick++.h>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <thread>

#define FFS_INODE_TABLE_TAG "ffs_inode"

std::string path_of(FFS::post_id_t id) {
	std::stringstream path_stream;
	path_stream << FFS_TMP_FS_PATH << "/ffs_" << id << "." << FFS_IMAGE_TYPE;
	return path_stream.str();
}

FFS::blobs_t FFS::Storage::blobs(FFS::Directory& dir) {
	std::stringbuf buf;
	std::basic_iostream stream(&buf);

	dir.serialize(stream);

	return FFS::encode(stream);
}

FFS::blobs_t FFS::Storage::blobs(FFS::InodeTable& table) {
	std::stringbuf buf;
	std::basic_iostream stream(&buf);

	table.serialize(stream);

	return FFS::encode(stream);
}

std::shared_ptr<FFS::Directory> FFS::Storage::dir_from_blobs(FFS::blobs_t blobs) {
	std::stringbuf buf;
	std::basic_iostream stream(&buf);

	FFS::decode(blobs, stream);

	stream.flush();
		
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

void FFS::Storage::update(std::shared_ptr<FFS::Directory> dir, FFS::inode_t inode) {
	auto new_post_id_ts = FFS::Storage::upload_file(FFS::Storage::blobs(*dir));
	auto table = FFS::State::get_inode_table();
	auto inode_entry = table->entry(inode);


	// remove old dir from storage device
	FFS::Storage::remove_posts(inode_entry->post_ids);
	inode_entry->post_ids = new_post_id_ts;
}

FFS::post_id_t FFS::Storage::upload_file(std::shared_ptr<Magick::Blob> blob, bool is_inode) {
	// Write to temporary file, upload, and then remove temp file


	auto tmp_filename = "/tmp/ffs_" + std::to_string(FFS::random_int());

	Magick::Image img(*blob);
	img.write(tmp_filename);

#ifdef DEBUG
	auto id = FFS::API::Local::save_file(tmp_filename, is_inode);
#elif
	std::string tag = "";
	if(is_inode) {
		tag = FFS_INODE_TABLE_TAG;
	}
	auto id = FFS::API::Flickr::post_image(tmp_filename, "", tag);
#endif

	std::filesystem::remove(tmp_filename);

	FFS::Cache::cache(id, blob);
	
	return id;
}

FFS::posts_t FFS::Storage::upload_file(FFS::blobs_t blobs) {
	auto posts = std::make_shared<std::vector<FFS::post_id_t>>();

	for(auto blob: *blobs) {
		FFS::post_id_t id = upload_file(blob);
		posts->push_back(id);
	}

	return posts;
}

std::shared_ptr<Magick::Blob> FFS::Storage::get_file(FFS::post_id_t id) {
	if(FFS::Cache::get(id) != nullptr)
		return FFS::Cache::get(id);

#ifdef DEBUG
	auto file_stream = FFS::API::Local::get_file(id);
#elif
	auto source_url = FFS::API::Flickr::get_image(id);
	auto file_stream = FFS::API::HTTP::get(source_url);
#endif

	// Read stream length
	auto length = FFS::stream_size(*file_stream);

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

FFS::blobs_t FFS::Storage::get_file(posts_t ids) {
	auto v = std::make_shared<std::vector<std::shared_ptr<Magick::Blob>>>();
	for(auto id: *ids) {
		v->push_back(get_file(id));
	}
	return v;
}

FFS::post_id_t FFS::Storage::get_inode_table() {
	std::string tag = FFS_INODE_TABLE_TAG;

#ifdef DEBUG
	auto post_id_t = FFS::API::Local::get_inode_post_id();
#elif
	auto post_id_t = FFS::API::Flickr::search_image(tag);
#endif

	return post_id_t;
}

void FFS::Storage::remove_post(FFS::post_id_t& post_id, bool multithread) {
	// Already deleting
	std::cout << "checking if deleing" << std::endl;
	if(FFS::State::is_deleting(post_id))
		return;
	std::cout << "Not delteing" << std::endl;
	FFS::State::deleting(post_id);
	std::cout << "Marked deleing" << std::endl;
	
	auto thread = std::thread([post_id] {
		try {
#ifdef DEBUG
			std::cout << "Deleting file" << std::endl;
			FFS::API::Local::delete_file(post_id);
#elif
			FFS::API::Flickr::delete_image(post_id);
#endif
		} catch(FFS::FlickrException& e) {
			std::cerr << "Could not delete post with id " << post_id << std::endl;
		}
		std::cout << "Mark deleted" << std::endl;
		FFS::State::deleted(post_id);
	});

	// If multithread is true, detach. Else, wait until done
	if(multithread)
		thread.detach();
	else if(thread.joinable()) {
		std::cout << "joining" <<std::endl;
		thread.join();
		std::cout << "thread done" <<std::endl;
	}

	std::cout << "INvalidate cache" << std::endl;
	FFS::Cache::invalidate(post_id);
}

void FFS::Storage::remove_posts(posts_t posts, bool multithread) {
	for(auto post_id: *posts) {
		remove_post(post_id, multithread);
	}
}
#include "../config.h"

#include "storage.h"

#include "file_coder.h"
#include "cache.h"

#include "../helpers/types.h"
#include "../helpers/functions.h"
#include "../helpers/constants.h"
#include "../helpers/logger.h"

#include "../system/state.h"

#include "../api/flickr.h"
#include "../api/curl.h"

#ifdef USE_LOCAL_STORAGE
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

#define FFS_INODE_TABLE_TAG "ffsinode"

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

std::shared_ptr<FFS::InodeTable> FFS::Storage::itable_from_blob(FFS::blob_t blob) {
	std::stringbuf buf;
	std::basic_iostream stream(&buf);

	auto blobs = std::make_shared<std::vector<FFS::blob_t>>();
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

FFS::post_id_t FFS::Storage::upload_file(FFS::blob_t blob, bool is_inode) {
	// Write to temporary file, upload, and then remove temp file

	auto tmp_filename = "/tmp/ffs_" + std::to_string(FFS::random_int());

	Magick::Image img(*blob);
	img.write(tmp_filename);

#ifdef USE_LOCAL_STORAGE
	auto id = FFS::API::Local::save_file(tmp_filename, is_inode);
#endif
#ifndef USE_LOCAL_STORAGE
	std::string tag = "";
	if(is_inode) {
		tag = FFS_INODE_TABLE_TAG;
	}
	auto id = FFS::API::Flickr::post_image(tmp_filename, tag);
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

FFS::blob_t as_blob(std::istream& stream) {
	// Read stream length
	auto length = FFS::stream_size(stream);

	// Copy stream to data array for Blob
	int index = 0;
	char* data = new char[length];
	while(stream) {
		FFS::read_c(stream, data[index++]);
	}

	auto blob = std::make_shared<Magick::Blob>(data, length);

	delete[] data;

	return blob;
}

FFS::blob_t FFS::Storage::get_file(FFS::post_id_t id) {
	if(FFS::Cache::get(id) != nullptr)
		return FFS::Cache::get(id);

#ifdef USE_LOCAL_STORAGE
	auto file_stream = FFS::API::Local::get_file(id);
#endif
#ifndef USE_LOCAL_STORAGE
	auto source_url = FFS::API::Flickr::get_image(id);
	auto file_stream = FFS::API::HTTP::get(source_url);
#endif

	auto blob = as_blob(*file_stream);

	FFS::Cache::cache(id, blob);

	return blob;
}

FFS::blobs_t FFS::Storage::get_file(posts_t ids) {
	auto v = std::make_shared<std::vector<FFS::blob_t>>();
	for(auto id: *ids) {
		v->push_back(get_file(id));
	}
	return v;
}

std::pair<FFS::blob_t, FFS::post_id_t&> FFS::Storage::get_inode_table() {
	std::string tag = FFS_INODE_TABLE_TAG;

#ifdef USE_LOCAL_STORAGE
	auto post_id = FFS::API::Local::get_inode_post_id();
	auto blob = get_file(post_id);
#endif
#ifndef USE_LOCAL_STORAGE
	auto search = FFS::API::Flickr::search_image(tag);
	auto post_id = search.post_id;
	auto url = search.url;

	auto image_stream = FFS::API::HTTP::get(url);
	auto blob = as_blob(*image_stream);
#endif

	return std::pair<FFS::blob_t, FFS::post_id_t&>(blob, post_id);
}

void FFS::Storage::remove_post(FFS::post_id_t& post_id, bool multithread) {
	// Already deleting
	if(FFS::State::is_deleting(post_id))
		return;
	FFS::State::deleting(post_id);
	
	auto func = [post_id] {
		try {
#ifdef USE_LOCAL_STORAGE
			FFS::API::Local::delete_file(post_id);
#endif
#ifndef USE_LOCAL_STORAGE
			FFS::API::Flickr::delete_image(post_id);
#endif
		} catch(FFS::FlickrException& e) {
			FFS::err << "Could not delete post with id " << post_id << std::endl;
		}
		FFS::State::deleted(post_id);
	};

	// If multithread is true, detach. Else, run on same thread
	if(multithread) 
		std::thread(func).detach();
	else
		func();

	FFS::Cache::invalidate(post_id);
}

void FFS::Storage::remove_posts(posts_t posts, bool multithread) {
	for(auto post_id: *posts) {
		remove_post(post_id, multithread);
	}
}
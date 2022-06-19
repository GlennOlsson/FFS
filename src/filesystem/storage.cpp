#include "storage.h"

#include "file_coder.h"

#include "../helpers/types.h"
#include "../helpers/functions.h"
#include "../helpers/constants.h"

#include "../system/state.h"

#include <Magick++.h>
#include <vector>
#include <string>
#include <sstream>

std::string path_of(FFS::post_id id) {
	std::stringstream path_stream;
	path_stream << FFS_TMP_FS_PATH << "/ffs_" << std::to_string(id) << "." << FFS_IMAGE_TYPE;
	return path_stream.str();
}


Magick::Blob* FFS::Storage::blob(FFS::Directory& dir) {
	std::stringbuf buf;
	std::basic_iostream stream(&buf);

	dir.sterilize(stream);

	uint32_t size = dir.size();

	return FFS::create_image(stream, size);
}

Magick::Blob* FFS::Storage::blob(FFS::InodeTable& table) {
	std::stringbuf buf;
	std::basic_iostream stream(&buf);

	table.sterilize(stream);

	uint32_t size = table.size();

	return FFS::create_image(stream, size);
}

FFS::Directory* FFS::Storage::dir_from_blob(Magick::Blob* blob) {
	std::stringbuf buf;
	std::basic_iostream stream(&buf);

	std::vector<Magick::Blob*>* v = new std::vector<Magick::Blob*>();
	v->push_back(blob);
	FFS::decode(v, stream);

	return FFS::Directory::desterilize(stream);
}

FFS::InodeTable* FFS::Storage::itable_from_blob(Magick::Blob* blob) {
	std::stringbuf buf;
	std::basic_iostream stream(&buf);

	std::vector<Magick::Blob*>* v = new std::vector<Magick::Blob*>();
	v->push_back(blob);
	FFS::decode(v, stream);

	return FFS::InodeTable::desterilize(stream);
}

void FFS::Storage::upload(FFS::Directory& dir) {
	// If -1, don't save in FS. Used for testing
	if(dir.self_id == -1)
		return;

	auto new_id = FFS::Storage::upload_file(FFS::Storage::blob(dir));
	auto table = FFS::State::get_inode_table();
	auto inode = table->entry(dir.self_id);
	
	// Free old list
	delete inode->post_blocks;
	inode->post_blocks = new std::vector<FFS::post_id>({new_id});

	FFS::State::save_table();
}

void FFS::Storage::save_file(FFS::post_id id, Magick::Blob* blob) { 
	std::string path = path_of(id);

	Magick::Image img(*blob);
	img.write(path);
}

FFS::post_id FFS::Storage::upload_file(Magick::Blob* blob) {
	// Assume no collision as it's 64-bit, i.e. 1.8e19 choices
	FFS::post_id id = FFS::random_long();
	save_file(id, blob);
	
	return id;
}

std::vector<FFS::post_id>* FFS::Storage::upload_file(std::vector<Magick::Blob*>* blobs) {
	std::vector<FFS::post_id>* posts = new std::vector<FFS::post_id>(blobs->size());

	for(Magick::Blob* blob: *blobs) {
		FFS::post_id id = upload_file(blob);
		posts->push_back(id);
	}

	return posts;
}

Magick::Blob* FFS::Storage::get_file(FFS::post_id id) {
	std::string path = path_of(id);
	Magick::Image img(path);

	Magick::Blob* blob = new Magick::Blob();
	img.write(blob);

	return blob;
}
#pragma once

#include "../helpers/types.h"

#include "directory.h"
#include "inode_table.h"

#include <string>
#include <vector>
#include <Magick++.h>

// API to save, get and delete FFS files on online services and where the data is stored
namespace FFS::Storage {

	/**
	 * @brief Get a Blob representing the directory as a FFS image
	 * 
	 * @return Magick::Blob* 
	 */
	Magick::Blob* blob(Directory&);
	Magick::Blob* blob(InodeTable&);

	Directory* dir_from_blob(Magick::Blob* blob);
	InodeTable* itable_from_blob(Magick::Blob* blob);

	void upload(FFS::Directory&);
	void upload(FFS::InodeTable&);

	void save_file(post_id id, Magick::Blob* blob);
	std::vector<post_id>* upload_file(std::vector<Magick::Blob*>* blobs);
	post_id upload_file(Magick::Blob* blob);
	Magick::Blob* get_file(post_id id);
};
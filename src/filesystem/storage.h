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
	std::vector<Magick::Blob*>* blobs(Directory&);
	std::vector<Magick::Blob*>* blobs(InodeTable&);

	Directory* dir_from_blobs(std::vector<Magick::Blob*>* blob);
	InodeTable* itable_from_blobs(std::vector<Magick::Blob*>* blobs);

	FFS::inode_id upload(Directory&);
	void update(Directory&, inode_id);

	void save_file(post_id id, Magick::Blob* blob);

	// Upload file and add to inode table. Return inode id in table
	FFS::inode_id upload_and_save_file(std::vector<Magick::Blob*>* blobs, bool is_dir = false);

	// Upload blobs but don't save to the inode table
	std::vector<FFS::post_id>* upload_file(std::vector<Magick::Blob*>* blobs);

	Magick::Blob* get_file(post_id id);
	std::vector<Magick::Blob*>* get_file(std::vector<post_id>* ids);
};
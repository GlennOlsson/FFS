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
	std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> blobs(Directory&);
	std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> blobs(InodeTable&);

	std::shared_ptr<Directory> dir_from_blobs(std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> blob);
	std::shared_ptr<InodeTable> itable_from_blobs(std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> blobs);

	// Upload new directory and save to inode table
	FFS::inode_id upload(std::shared_ptr<Directory>);
	
	// Update existing directory with new blocks and save inode table
	void update(Directory&, inode_id);

	void save_file(post_id id, std::shared_ptr<Magick::Blob> blob);

	// Upload file and add to inode table. Return inode id in table
	FFS::inode_id upload_and_save_file(std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> blobs, size_t size, bool is_dir = false);

	// Upload blobs but don't save to the inode table
	std::shared_ptr<std::vector<FFS::post_id>> upload_file(std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> blobs);

	std::shared_ptr<Magick::Blob> get_file(post_id id);
	std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> get_file(std::shared_ptr<std::vector<post_id>> ids);

	// Remove the blocks from the storage media
	void remove_blocks(std::vector<FFS::post_id>& blocks);
};
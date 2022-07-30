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
	std::shared_ptr<InodeTable> itable_from_blob(std::shared_ptr<Magick::Blob> blob);
	
	// Update existing directory with new blocks and save inode table
	void update(std::shared_ptr<Directory>, inode_id);

	// Upload blob and return post_id. Blob is cached
	FFS::post_id upload_file(std::shared_ptr<Magick::Blob> blob, bool is_inode = false);
	// Upload blobs but don't save to the inode table
	std::shared_ptr<std::vector<FFS::post_id>> upload_file(std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> blobs);

	// Get file from service
	std::shared_ptr<Magick::Blob> get_file(post_id id);
	std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> get_file(std::shared_ptr<std::vector<post_id>> ids);
	
	// Get the inode table from the storage medium
	FFS::post_id get_inode_table();

	// Remove a post from the storage media
	void remove_post(FFS::post_id& post);

	// Remove the posts from the storage media
	void remove_posts(std::vector<FFS::post_id>& posts);
};
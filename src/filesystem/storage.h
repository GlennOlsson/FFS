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
	FFS::blobs_t blobs(Directory&);
	FFS::blobs_t blobs(InodeTable&);

	std::shared_ptr<Directory> dir_from_blobs(FFS::blobs_t blob);
	std::shared_ptr<InodeTable> itable_from_blob(std::shared_ptr<Magick::Blob> blob);
	
	// Update existing directory with new blocks and save inode table
	void update(std::shared_ptr<Directory>, inode_t);

	// Upload blob and return post_id_t. Blob is cached
	FFS::post_id_t upload_file(std::shared_ptr<Magick::Blob> blob, bool is_inode = false);
	// Upload blobs but don't save to the inode table
	posts_t upload_file(FFS::blobs_t blobs);

	// Get file from service
	std::shared_ptr<Magick::Blob> get_file(post_id_t id);
	FFS::blobs_t get_file(posts_t ids);
	
	// Get the inode table from the storage medium
	FFS::post_id_t get_inode_table();

	// Remove a post from the storage media. Set _wait_ if should wait for delete to finish before returning
	void remove_post(FFS::post_id_t& post, bool wait = false);

	// Remove the posts from the storage media
	void remove_posts(posts_t posts);
};
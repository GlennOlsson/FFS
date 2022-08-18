#include "fs.h"

#include "directory.h"
#include "storage.h"
#include "file_coder.h"
#include "inode_table.h"
#include "cache.h"

#include "../system/state.h"
#include "../helpers/constants.h"
#include "../helpers/functions.h"
#include "../helpers/logger.h"
#include "../exceptions/exceptions.h"

#include <string>
#include <sstream>
#include <ostream>
#include <vector>
#include <iostream>
#include <memory>
#include <thread>
#include <exception>
#include <Magick++.h>

// Expected in the form 
//  	foo/bar/fizz[ext]
// or
//		/for/bar/fizz[ext]
// TODO: Test
std::vector<std::string> path_parts(std::string path) {
	std::vector<std::string> v;
	size_t prev_pos = 0;
	size_t pos = path.find("/");
	if(pos == 0) { // if first char, skip it as to not fuck up dir list
		path = path.substr(1);
		pos = path.find("/");
	}

	while(pos != std::string::npos) {
		size_t len = pos - prev_pos;
		std::string s = path.substr(prev_pos, len);

		v.push_back(s);

		prev_pos = pos + 1; // Skip the /
		pos = path.find("/", pos + 1);
	}

	std::string filename = path.substr(prev_pos);
	v.push_back(filename);

	return v;
}

std::shared_ptr<FFS::Directory> get_root_dir() {
	auto cached_root = FFS::Cache::get_root();
	if(cached_root != nullptr)
		return cached_root;

	auto table = FFS::State::get_inode_table();

	std::shared_ptr<FFS::Directory> dir;

	try {
		dir = FFS::FS::get_dir(FFS_ROOT_INODE);
	} catch(FFS::NoPhotoWithID) {
		// if the root dir had error fetching, replace with new root dir in table
		FFS::err << "Could not get root dir" << std::endl;
		dir = std::make_shared<FFS::Directory>();
		table->entries->insert_or_assign(FFS_ROOT_INODE, dir);
	}

	FFS::Cache::cache_root(dir);

	return dir;
}

// traverse a tree of directories, returning a structure containing the parent directory and its inode id, along
// with the filename (could be a directory name too).
//
// No check is made if the file exists in the specified path
std::shared_ptr<FFS::FS::Traverser> FFS::FS::traverse_path(std::string path) {
	//Special case for root
	if(path == "/") {
		auto root = get_root_dir();
		return std::make_shared<FFS::FS::Traverser>(FFS::FS::Traverser({root, FFS_ROOT_INODE, "/", "/"}));
	}

	std::vector<std::string> dirs = path_parts(path);
	std::string filename = dirs.back();
	dirs.pop_back(); // Removes last element == filename

	// Start with dir as root dir
	auto dir = get_root_dir();

	auto table = FFS::State::get_inode_table();
	std::shared_ptr<FFS::InodeEntry> dir_entry = nullptr;

	FFS::inode_t inode = FFS_ROOT_INODE;
	for(std::string dir_name: dirs) {
        inode = dir->get_file(dir_name);
		dir_entry = table->entry(inode);
		if(!dir_entry->is_dir) {
			throw FFS::BadFFSPath(path, dir_name);
		}
		dir = FFS::FS::get_dir(inode);
	}

	return std::make_shared<FFS::FS::Traverser>(FFS::FS::Traverser({dir, inode, filename, path}));
}

// Check if file is in the parent directory of a traverser object. Throws NoPathWithName if not in
void FFS::FS::verify_in(std::shared_ptr<FFS::FS::Traverser> tr) {
    if(!tr->parent_dir->entries->contains(tr->filename)) {
        throw FFS::NoPathWithName(tr->full_path);
    }
}

// Check that file is NOT in the parent directory of a traverser object. Throws FileAlreadyExists if not in
void FFS::FS::verify_not_in(std::shared_ptr<FFS::FS::Traverser> tr) {
    if(tr->parent_dir->entries->contains(tr->filename)) {
        throw FFS::FileAlreadyExists(tr->filename);
    }
}

// Remove the trailing slash (eg. /foo/bar/ becomes /foo/bar) if it exists 
void remove_trailing_slash(std::string& s) {
	if(s.back() == '/')
		s.pop_back();
}

FFS::inode_t inode_from_path(std::string path) {
	auto table = FFS::State::get_inode_table();

	if(path == "/")
		return FFS_ROOT_INODE;

	remove_trailing_slash(path);

	auto traverser = FFS::FS::traverse_path(path);
    verify_in(traverser);

	auto filename = traverser->filename;
    auto dir = traverser->parent_dir;

	auto inode = dir->get_file(filename);
	return inode;
}

std::shared_ptr<FFS::InodeEntry> FFS::FS::entry(FFS::inode_t inode) {
	auto table = FFS::State::get_inode_table();
	return table->entry(inode);
}

std::shared_ptr<FFS::InodeEntry> FFS::FS::entry(std::string path) {
	auto inode = inode_from_path(path);
	return entry(inode);
}

std::shared_ptr<FFS::Directory> FFS::FS::read_dir(std::string path) {
	// special case for root dir, /
	if(path == "/")
		return get_root_dir();
	
	remove_trailing_slash(path);

	auto inode = inode_from_path(path);

    auto inode_entry = entry(inode);
    return get_dir(inode);
}

std::shared_ptr<FFS::Directory> FFS::FS::get_dir(std::shared_ptr<FFS::InodeEntry> inode_entry) {
	// If there are no post blocks it is an empty directory
	if(inode_entry->post_ids == nullptr)
		return std::make_shared<FFS::Directory>();

	auto blobs = FFS::Storage::get_file(inode_entry->post_ids);
	auto dir = FFS::Storage::dir_from_blobs(blobs);
    
	return dir;
}

std::shared_ptr<FFS::Directory> FFS::FS::get_dir(FFS::inode_t inode) {
	auto inode_entry = entry(inode);
	return get_dir(inode_entry);
}

void FFS::FS::read_file(FFS::inode_t inode, std::ostream& stream) {
	auto inode_entry = entry(inode);
	// If is nullptr, empty file
	if(inode_entry->post_ids == nullptr)
		return;

	auto blobs = FFS::Storage::get_file(inode_entry->post_ids);

	FFS::decode(blobs, stream);
}

void FFS::FS::read_file(std::string path, std::ostream& stream) {
	auto inode = inode_from_path(path);

	read_file(inode, stream);
}

std::shared_ptr<std::pair<FFS::inode_t, std::shared_ptr<FFS::Directory>>> FFS::FS::parent_entry(std::string path) {
	auto table = FFS::State::get_inode_table();

	if(path == "/") {
		auto root_dir = get_root_dir();

		return std::make_shared<std::pair<FFS::inode_t, std::shared_ptr<FFS::Directory>>>
			(FFS_ROOT_INODE, root_dir);
	}
	
	remove_trailing_slash(path);

	auto traverser = traverse_path(path);
	// Don't verify that it's in, doesn't have to (eg. ffs_rename)

	return std::make_shared<std::pair<FFS::inode_t, std::shared_ptr<FFS::Directory>>>
		(traverser->parent_inode, traverser->parent_dir);
}

std::string FFS::FS::filename(std::string path) {
	if(path == "/")
		return path;

	remove_trailing_slash(path);

	auto parts = path_parts(path);
	// If there are no parts, i.e. not even a filename, the file does not exist (because it could only have been the root dir)
	if(parts.size() == 0)
		throw FFS::NoFileWithName(path);

	return parts.back();
}

void FFS::FS::create_dir(std::string path) {
	remove_trailing_slash(path);
	
	auto traverser = traverse_path(path);
	verify_not_in(traverser);

	auto dir_name = traverser->filename;
    auto parent_dir = traverser->parent_dir;
	auto parent_inode = traverser->parent_inode;

	// Add empty entry to inode table
	auto table = FFS::State::get_inode_table();
	auto new_dir_inode = table->new_file(nullptr, 0, true);

	parent_dir->add_entry(dir_name, new_dir_inode);
	FFS::Storage::update(parent_dir, parent_inode);
}

FFS::blobs_t FFS::FS::update_file(FFS::inode_t inode, std::istream& stream) {
	auto stream_length = FFS::stream_size(stream);
	auto blobs = FFS::encode(stream);

	auto inode_table = FFS::State::get_inode_table();
	auto entry = inode_table->entry(inode);

	// Remove old posts
	auto old_posts = entry->post_ids;
	if(old_posts != nullptr)
		FFS::Storage::remove_posts(old_posts);

	entry->length = stream_length;
	
	auto now = FFS::curr_milliseconds();
	entry->time_modified = now;
	entry->time_accessed = now;

	return blobs;
}

void FFS::FS::create_file(std::string path, std::shared_ptr<std::istream> stream) {
	remove_trailing_slash(path);
	
	auto traverser = traverse_path(path);
	verify_not_in(traverser);

	auto filename = traverser->filename;
    auto parent_dir = traverser->parent_dir;
	auto parent_inode = traverser->parent_inode;

	// If stream is nullptr, create as empty file in inode table. Means that is does not have to be uploaded
	std::shared_ptr<std::__1::vector<FFS::post_id_t>> posts = nullptr;
	size_t stream_length = 0;
	if(stream != nullptr) {
		stream_length = FFS::stream_size(*stream);
		auto blobs = FFS::encode(*stream);
		posts = FFS::Storage::upload_file(blobs);
	}

	auto inode_table = FFS::State::get_inode_table();
	auto inode = inode_table->new_file(posts, stream_length, false);

	parent_dir->add_entry(filename, inode);

	// Save new content of parent dir
	FFS::Storage::update(parent_dir, parent_inode);
}

// Traverse through all sub directories and add all posts to fill_v, and remove all inodes from inode table
void remove_sub_files(FFS::posts_t fill_v, std::shared_ptr<FFS::Directory> dir) {
	auto table = FFS::State::get_inode_table();

	for(auto dir_entry: *dir->entries) {
		auto file_inode = dir_entry.second;
		auto file_entry = table->entry(file_inode);
		auto file_posts = file_entry->post_ids;

		if(file_entry->is_dir) {
			auto dir = FFS::FS::get_dir(file_inode);

			remove_sub_files(fill_v, dir);
		} 
		
		table->remove_entry(file_inode);

		// Insert new posts to be removed
		fill_v->insert(fill_v->begin(), file_posts->begin(), file_posts->end());
	}
}

void FFS::FS::remove(std::string path, bool multithread) {
	remove_trailing_slash(path);
	
	auto traverser = traverse_path(path);
    verify_in(traverser);

	auto filename = traverser->filename;
    auto parent_dir = traverser->parent_dir;
	auto parent_inode = traverser->parent_inode;

	auto inode = parent_dir->remove_entry(filename);

	// Update parent on other thread
	auto thread = std::thread([parent_dir, parent_inode] {
		FFS::Storage::update(parent_dir, parent_inode);
	});
	if(multithread)
		thread.detach();
	else
		thread.join();

	auto table = FFS::State::get_inode_table();
	
	auto entry = table->entry(inode);
	auto posts_to_remove = entry->post_ids;

	if(entry->is_dir) {
		auto dir = FFS::FS::get_dir(inode);
		remove_sub_files(posts_to_remove, dir);
	}

	// remove from storage device. Made on different thread
	FFS::Storage::remove_posts(posts_to_remove, multithread);

	table->remove_entry(inode);
}

bool FFS::FS::exists(std::string path) {
	// Root always exists
	if(path == "/")
		return true;
	remove_trailing_slash(path);
	
	try {
		auto traverser = traverse_path(path);
		return traverser->parent_dir->entries->count(traverser->filename) > 0;
	} catch(FFS::BadFFSPath) {
		return false;
	} catch(FFS::NoFileWithName) { // Thrown if parent dir does not exist
		return false;
	} catch(std::exception& e) {
		return false;
	}
}

bool FFS::FS::is_dir(std::string path) {
	// Root is always a dir
	if(path == "/")
		return true;

	remove_trailing_slash(path);
	
	auto traverser = traverse_path(path);
	verify_in(traverser);

	auto filename = traverser->filename;
    auto parent_dir = traverser->parent_dir;

	auto inode = parent_dir->get_file(filename);

	auto table = FFS::State::get_inode_table();
	auto entry = table->entry(inode);

	return entry->is_dir;
}

void FFS::FS::sync_inode_table() {
	FFS::State::save_table();
}
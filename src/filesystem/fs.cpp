#include "fs.h"

#include "directory.h"
#include "storage.h"
#include "file_coder.h"
#include "inode_table.h"

#include "../system/state.h"
#include "../helpers/constants.h"
#include "../helpers/functions.h"
#include "../exceptions/exceptions.h"

#include <string>
#include <sstream>
#include <ostream>
#include <vector>
#include <iostream>
#include <memory>

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
struct Traverser {
	std::shared_ptr<FFS::Directory> dir;
	FFS::inode_id dir_inode;
	std::string filename;

    std::string full_path;
};

std::shared_ptr<FFS::Directory> get_root_dir() {
	auto table = FFS::State::get_inode_table();
	// Root dir entry
	auto dir_entry = table->entry(FFS_ROOT_INODE);
	auto blobs = FFS::Storage::get_file(dir_entry->post_blocks);

	auto dir = FFS::Storage::dir_from_blobs(blobs);
	return dir;
}

// traverse a tree of directories, returning a structure containing the parent directory and its inode id, along
// with the filename (could be a directory name too).
//
// No check is made if the file exists in the specified path
std::shared_ptr<struct Traverser> traverse_path(std::string path) {
	std::vector<std::string> dirs = path_parts(path);
	std::string filename = dirs.back();
	dirs.pop_back(); // Removes last element == filename

	// Start with dir as root dir
	auto dir = get_root_dir();

	auto table = FFS::State::get_inode_table();
	auto blobs = std::make_shared<std::vector<std::shared_ptr<Magick::Blob>>>();
	auto dir_entry = std::make_shared<FFS::InodeEntry>();
	FFS::inode_id inode_id = FFS_ROOT_INODE;
	for(std::string dir_name: dirs) {
        inode_id = dir->get_file(dir_name);
        dir_entry = table->entry(inode_id);
        if(!dir_entry->is_dir) {
            throw FFS::BadFFSPath(path, dir_name);
        }
        blobs = FFS::Storage::get_file(dir_entry->post_blocks);
        dir = FFS::Storage::dir_from_blobs(blobs);
	}

	return std::make_shared<struct Traverser>(dir, inode_id, filename, path);
}

// Check if file is in the parent directory of a traverser object. Throws BadFFSPath if not in
void verify_file_in(std::shared_ptr<struct Traverser> tr) {
    if(!tr->dir->entries->contains(tr->filename)) {
        throw FFS::BadFFSPath(tr->full_path, tr->filename);
    }
}

// Check that file is not in the parent directory of a traverser object. Throws BadFFSPath if not in
void verify_not_in(std::shared_ptr<struct Traverser> tr) {
    if(tr->dir->entries->contains(tr->filename)) {
        throw FFS::FileAlreadyExists(tr->filename);
    }
}

// Remove the trailing slash (eg. /foo/bar/ becomes /foo/bar) if it exists 
void remove_trailing_slash(std::string& s) {
	if(s.back() == '/')
		s.pop_back();
}

std::shared_ptr<FFS::Directory> FFS::FS::read_dir(std::string path) {
	// special case for root dir, /
	if(path == "/")
		return get_root_dir();
	
	remove_trailing_slash(path);

    auto inode_entry = entry(path);
    auto blobs = FFS::Storage::get_file(inode_entry->post_blocks);
    return FFS::Storage::dir_from_blobs(blobs);
}

void FFS::FS::read_file(std::string path, std::ostream& stream) {
	auto inode_entry = entry(path);
    auto blobs = FFS::Storage::get_file(inode_entry->post_blocks);

	FFS::decode(blobs, stream);
}

std::shared_ptr<FFS::InodeEntry> FFS::FS::entry(std::string path) {
	auto table = FFS::State::get_inode_table();

	if(path == "/")
		return table->entry(FFS_ROOT_INODE);

	remove_trailing_slash(path);

	auto traverser = traverse_path(path);
    verify_file_in(traverser);

	auto filename = traverser->filename;
    auto dir = traverser->dir;

	auto inode_entry = table->entry(dir->get_file(filename));
	return inode_entry;
}

void FFS::FS::create_dir(std::string path) {
	remove_trailing_slash(path);
	
	auto traverser = traverse_path(path);
	verify_not_in(traverser);

	auto dir_name = traverser->filename;
    auto parent_dir = traverser->dir;
	auto parent_inode = traverser->dir_inode;

	auto dir = std::shared_ptr<Directory>();
	auto inode = FFS::Storage::upload(*dir);
	parent_dir->add_entry(dir_name, inode);
	FFS::Storage::update(*parent_dir, parent_inode);
}

void FFS::FS::create_file(std::string path, std::istream& stream) {
	remove_trailing_slash(path);
	
	auto traverser = traverse_path(path);
	verify_not_in(traverser);

	auto filename = traverser->filename;
    auto parent_dir = traverser->dir;
	auto parent_inode = traverser->dir_inode;

	auto blobs = FFS::encode(stream);

	// Adds it to inode table
	auto inode_id = FFS::Storage::upload_and_save_file(blobs, FFS::stream_size(stream));

	parent_dir->add_entry(filename, inode_id);

	// Save new content of parent dir
	FFS::Storage::update(*parent_dir, parent_inode);
}

void FFS::FS::remove(std::string path) {
	remove_trailing_slash(path);
	
	auto traverser = traverse_path(path);
    verify_file_in(traverser);

	auto filename = traverser->filename;
    auto parent_dir = traverser->dir;
	auto parent_inode = traverser->dir_inode;

	auto inode = parent_dir->remove_entry(filename);

	FFS::Storage::update(*parent_dir, parent_inode);
	
	auto table = FFS::State::get_inode_table();
	table->remove_entry(inode);
}

bool FFS::FS::exists(std::string path) {
	// Root always exists
	if(path == "/")
		return true;
	remove_trailing_slash(path);
	
	try {
		auto traverser = traverse_path(path);
		return traverser->dir->entries->count(traverser->filename) > 0;
	} catch(FFS::BadFFSPath) {
		return false;
	} catch(FFS::NoFileWithName) { // Thrown if parent dir does not exist
		return false;
	}
}

bool FFS::FS::is_dir(std::string path) {
	// Root is always a dir
	if(path == "/")
		return true;

	remove_trailing_slash(path);
	
	auto traverser = traverse_path(path);
	verify_file_in(traverser);

	auto filename = traverser->filename;
    auto parent_dir = traverser->dir;

	auto inode = parent_dir->get_file(filename);

	auto table = FFS::State::get_inode_table();
	auto entry = table->entry(inode);

	return entry->is_dir;
}
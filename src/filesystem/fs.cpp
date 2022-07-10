#include "fs.h"

#include "directory.h"
#include "storage.h"
#include "file_coder.h"

#include "../system/state.h"
#include "../helpers/constants.h"
#include "../exceptions/exceptions.h"

#include <string>
#include <sstream>
#include <ostream>
#include <vector>

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
	FFS::Directory* dir;
	FFS::inode_id dir_inode;
	std::string filename;

    std::string full_path;
};

// traverse a tree of directories, returning a structure containing the parent directory and its inode id, along
// with the filename (could be a directory name too).
//
// No check is made if the file exists in the specified path
struct Traverser* traverse_path(std::string path) {
	std::vector<std::string> dirs = path_parts(path);
	std::string filename = dirs.back();
	dirs.pop_back(); // Removes last element == filename

	FFS::InodeTable* table = FFS::State::get_inode_table();

	// Root dir entry
	FFS::InodeEntry* dir_entry = table->entry(FFS_ROOT_INODE);
	// Assumes directory is only 1 post
	auto blobs = FFS::Storage::get_file(dir_entry->post_blocks);
	// Root dir (/)
	FFS::Directory* dir = FFS::Storage::dir_from_blobs(blobs);

	FFS::inode_id inode_id = FFS_ROOT_INODE;;
	for(std::string dir_name: dirs) {
        inode_id = dir->get_file(dir_name);
        dir_entry = table->entry(inode_id);
        if(!dir_entry->is_dir) {
            throw FFS::BadFFSPath(path, dir_name);
        }
        blobs = FFS::Storage::get_file(dir_entry->post_blocks);
        dir = FFS::Storage::dir_from_blobs(blobs);
	}

	return new struct Traverser({dir, inode_id, filename, path});
}

// Check if file is in the parent directory of a traverser object. Throws BadFFSPath if not in
void verify_file_in(struct Traverser* tr) {
    if(!tr->dir->entries->contains(tr->filename)) {
        throw FFS::BadFFSPath(tr->full_path, tr->filename);
    }
}

FFS::Directory* FFS::FS::read_dir(std::string path) {
    auto traverser = traverse_path(path);
    verify_file_in(traverser);

    auto table = FFS::State::get_inode_table();
    auto filename = traverser->filename;
    auto dir = traverser->dir;

    auto inode_entry = table->entry(dir->get_file(filename));
    auto blobs = FFS::Storage::get_file(inode_entry->post_blocks);
    return FFS::Storage::dir_from_blobs(blobs);
}

std::istream* FFS::FS::read_file(std::string path) {
	auto traverser = traverse_path(path);
    verify_file_in(traverser);

	auto table = FFS::State::get_inode_table();
	auto filename = traverser->filename;
    auto dir = traverser->dir;

	auto inode_entry = table->entry(dir->get_file(filename));
    auto blobs = FFS::Storage::get_file(inode_entry->post_blocks);
	
	std::stringbuf* buf = new std::stringbuf();;
	std::basic_iostream<char>* stream = new std::basic_iostream<char>(buf);

	FFS::decode(blobs, *stream);

	return stream;
}

void FFS::FS::create_dir(std::string path) {
	auto traverser = traverse_path(path);
    verify_file_in(traverser);

	auto dir_name = traverser->filename;
    auto parent_dir = traverser->dir;
	auto parent_inode = traverser->dir_inode;

	auto dir = new Directory();
	auto inode = FFS::Storage::upload(*dir);
	parent_dir->add_entry(dir_name, inode);
	FFS::Storage::update(*parent_dir, parent_inode);
}

void FFS::FS::create_file(std::string path, std::istream* stream) {
	auto traverser = traverse_path(path);
    verify_file_in(traverser);

	auto filename = traverser->filename;
    auto parent_dir = traverser->dir;
	auto parent_inode = traverser->dir_inode;

	auto blobs = FFS::encode(*stream);

	// Adds it to inode table
	auto inode_id = FFS::Storage::upload_and_save_file(blobs);

	parent_dir->add_entry(filename, inode_id);

	// Save new content of parent dir
	FFS::Storage::update(*parent_dir, parent_inode);
}

void FFS::FS::remove(std::string path) {
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
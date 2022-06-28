#include "fs.h"

#include "directory.h"
#include "storage.h"

#include "../system/state.h"
#include "../helpers/constants.h"
#include "../exceptions/exceptions.h"

#include <string>
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
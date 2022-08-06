#include "file_handle.h"

#include "fs.h"

#include "../helpers/types.h"

#include "../exceptions/exceptions.h"

#include <map>
#include <string>

class FileHandler {
public:
	// When created, open has been called once
	size_t open_calls = 1;

	std::string filename;
	FFS::inode_t parent_inode;

	FileHandler(std::string filename, FFS::inode_t parent) : 
		filename(filename), parent_inode(parent) {}

	void open() {
		open_calls++;
	}
	// Close filehandle. Returns true if it was the last close
	bool close() {
		return --open_calls == 0;
	}
};

FFS::file_handle_t fh(FFS::inode_t inode) {
	return (FFS::file_handle_t) inode;
}

std::map<FFS::inode_t, FileHandler> open_files;

FFS::file_handle_t FFS::FileHandle::open(std::string path) {
	auto traverser = FFS::FS::traverse_path(path);

	FFS::FS::verify_in(traverser);

	auto filename = traverser->filename;
	auto inode = traverser->parent_dir->get_file(filename);

	// If call to already open file
	if(open_files.contains(inode)) {
		open_files.at(inode).open();
	} else { // If call to un-opened file
		auto parent_inode = traverser->parent_inode;
		open_files.insert({inode, FileHandler(filename, parent_inode)});
	}

	return fh(inode);
}

void FFS::FileHandle::close(FFS::file_handle_t fh) {
	auto inode = FFS::FileHandle::inode(fh);

	if(!open_files.contains(inode))
		throw FFS::NoOpenFileWithFH(fh);

	bool last_close = open_files.at(inode).close();
	if(last_close) {
		// Save open file/dir, parent dir and inode table

	}
}

FFS::inode_t FFS::FileHandle::inode(FFS::file_handle_t fh) {
	return (FFS::inode_t) fh;
}

FFS::inode_t FFS::FileHandle::parent(FFS::file_handle_t fh) {
	auto inode = FFS::FileHandle::inode(fh);
	return open_files.at(inode).parent_inode;
}	

std::string FFS::FileHandle::filename(FFS::file_handle_t fh) {
	auto inode = FFS::FileHandle::inode(fh);
	return open_files.at(inode).filename;
}

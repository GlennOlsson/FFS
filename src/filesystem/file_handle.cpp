#include "file_handle.h"

#include "../helpers/types.h"

#include <map>
#include <string>

class FileHandle {
public:
	size_t open_calls = 0;

	std::string filename;
	FFS::inode_id parent_inode;

	FileHandle(std::string filename, FFS::inode_id parent) : 
		filename(filename), parent_inode(parent) {}

	void open() {
		open_calls++;
	}
	// Close filehandle. Returns true if it was the last close
	bool close() {
		return --open_calls == 0;
	}
};

std::map<FFS::inode_id, FileHandle&> open_files;

file_handle_t FFS::FileHandle::open(std::string path) {
	
}

void FFS::FileHandle::close(file_handle_t) {

}

FFS::inode_id FFS::FileHandle::inode(file_handle_t fh) {
	return (FFS::inode_id) fh;
}

FFS::inode_id FFS::FileHandle::parent(file_handle_t fh) {
	auto inode = FFS::FileHandle::inode(fh);
	return open_files.at(inode).parent_inode;
}	

std::string FFS::FileHandle::filename(file_handle_t fh) {
	auto inode = FFS::FileHandle::inode(fh);
	return open_files.at(inode).filename;
}

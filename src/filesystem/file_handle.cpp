#include "file_handle.h"

#include "fs.h"
#include "storage.h"

#include "../helpers/types.h"
#include "../helpers/constants.h"

#include "../system/state.h"

#include "../exceptions/exceptions.h"

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <Magick++.h>

class FileHandler {
private:

	typedef std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> blobs_t;
	// When created, open has been called once
	size_t open_calls = 1;

	std::string filename;
	FFS::inode_t parent_inode;

	blobs_t blobs;
	blobs_t parent_blobs;

	bool modified = false;

public:

	FileHandler(std::string filename, FFS::inode_t parent) : 
		filename(filename), parent_inode(parent) {}

	void open() {
		open_calls++;
	}
	// Close filehandle. Returns true if it was the last close
	bool close() {
		return --open_calls == 0;
	}

	FFS::inode_t parent() {
		return this->parent_inode;
	}

	void set_blobs(blobs_t blobs) {
		this->blobs = blobs;
	}

	blobs_t get_blobs() {
		return this->blobs;
	}

	void set_parent_blobs(blobs_t blobs) {
		this->parent_blobs = blobs;
	}

	blobs_t get_parent_blobs() {
		return this->parent_blobs;
	}

	void modify() {
		this->modified = true;
	}

	bool is_modified() {
		return this->modified;
	}
};

std::size_t replace_all(std::string& inout, std::string_view what, std::string_view with) {
	std::size_t count{};
	for (std::string::size_type pos{};
		 inout.npos != (pos = inout.find(what.data(), pos, what.length()));
		 pos += with.length(), ++count) {
		inout.replace(pos, what.length(), with.data(), with.length());
	}
	return count;
}

std::string sanitize_path(std::string str) {
	replace_all(str, "\\ ", " ");
	replace_all(str, "ö", "ö"); // force replace special ö inserted my macos

	return str;
}

FFS::file_handle_t fh(FFS::inode_t inode) {
	return (FFS::file_handle_t) inode;
}

std::map<FFS::inode_t, FileHandler> open_files;

FFS::file_handle_t FFS::FileHandle::open(std::string path) {
	auto traverser = FFS::FS::traverse_path(sanitize_path(path));

	auto filename = traverser->filename;

	FFS::inode_t inode;
	// Special case if root
	if(path == "/") {
		inode = FFS_ROOT_INODE;
	} else {
		FFS::FS::verify_in(traverser);
		inode = traverser->parent_dir->get_file(filename);
	}

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

	auto& open_file = open_files.at(inode);

	bool last_close = open_file.close();
	if(last_close) {
		if(open_file.is_modified()) {
			// Save open file/dir, (parent dir?) and inode table
			auto table = FFS::State::get_inode_table();

			auto inode_entry = table->entry(inode);

			auto blobs = open_file.get_blobs();
			auto posts = FFS::Storage::upload_file(blobs);
			inode_entry->post_blocks = posts;

			FFS::State::save_table();
		}

		open_files.erase(inode);
	}
}

FFS::inode_t FFS::FileHandle::inode(FFS::file_handle_t fh) {
	return (FFS::inode_t) fh;
}

FFS::inode_t FFS::FileHandle::parent(FFS::file_handle_t fh) {
	auto inode = FFS::FileHandle::inode(fh);
	return open_files.at(inode).parent();
}	

// TODO: needed?
// std::string FFS::FileHandle::filename(FFS::file_handle_t fh) {
// 	auto inode = FFS::FileHandle::inode(fh);
// 	return open_files.at(inode).filename;
// }

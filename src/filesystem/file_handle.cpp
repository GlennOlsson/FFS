#include "file_handle.h"

#include "fs.h"
#include "storage.h"
#include "file_coder.h"

#include "../helpers/types.h"
#include "../helpers/functions.h"
#include "../helpers/constants.h"
#include "../helpers/logger.h"

#include "../system/state.h"

#include "../exceptions/exceptions.h"

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <Magick++.h>
#include <iostream>
#include <sstream>
#include <iostream>

class FileHandler {
private:
	// When created, open has been called once
	size_t open_calls = 1;

	std::string filename;
	FFS::inode_t parent_inode;

	std::shared_ptr<std::iostream> stream;

	size_t size;

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

	void set_stream(std::shared_ptr<std::iostream> stream) {
		this->stream = stream;
		this->modified = true;
	}

	std::shared_ptr<std::iostream> get_stream() {
		return this->stream;
	}

	void set_size(size_t size) {
		this->size = size;
	}

	size_t get_size() {
		return this->size;
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

FileHandler& get_open_file(FFS::inode_t inode) {
	if(!open_files.contains(inode))
		throw FFS::NoOpenFileWithFH(fh(inode));
	
	return open_files.at(inode);
}

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
		auto& file_handler = get_open_file(inode);
		file_handler.open();
	} else { // If call to un-opened file
		auto parent_inode = traverser->parent_inode;
		auto file_handler = FileHandler(filename, parent_inode);
		
		// At first open, set the size at the current size of the file or dir
		auto table = FFS::State::get_inode_table();
		auto inode_entry = table->entry(inode);
		file_handler.set_size(inode_entry->length);

		open_files.insert({inode, file_handler});
	}

	return fh(inode);
}

FFS::file_handle_t FFS::FileHandle::create(std::string path) {
	auto traverser = FFS::FS::traverse_path(sanitize_path(path));

	auto filename = traverser->filename;

	FFS::FS::verify_not_in(traverser);
	
	auto table = FFS::State::get_inode_table();

	auto inode = table->new_file(nullptr, 0, false);

	auto parent_inode = traverser->parent_inode;
	auto file_handle = FileHandler(filename, parent_inode);

	auto buf = std::make_shared<std::stringbuf>();
	auto stream = std::make_shared<std::basic_iostream<char>>(buf.get());

	file_handle.set_stream(stream);
	open_files.insert({inode, file_handle});

	auto parent_dir = traverser->parent_dir;
	parent_dir->add_entry(filename, inode);

	// Save new content of parent dir
	FFS::Storage::update(parent_dir, parent_inode);

	return fh(inode);
}

void FFS::FileHandle::close(FFS::file_handle_t fh) {
	auto inode = FFS::FileHandle::inode(fh);

	if(!open_files.contains(inode))
		throw FFS::NoOpenFileWithFH(fh);

	auto& open_file = get_open_file(inode);

	bool last_close = open_file.close();
	if(last_close) {
		if(open_file.is_modified()) {
			// Save open file/dir, (parent dir?) and inode table
			auto table = FFS::State::get_inode_table();

			auto inode_entry = table->entry(inode);

			// Remove the current posts of the file
			auto current_posts = inode_entry->post_ids;

			// auto blobs = open_file.get_blobs();
			auto stream = open_file.get_stream();
			auto blobs = FFS::encode(*stream);
			posts_t posts = nullptr;
			if(blobs != nullptr) 
				posts = FFS::Storage::upload_file(blobs);
			//Else posts will be nullptr

			inode_entry->post_ids = posts;

			auto now = FFS::curr_milliseconds();		
			inode_entry->time_modified = now;
			inode_entry->time_accessed = now;

			inode_entry->length = open_file.get_size();

			FFS::Storage::remove_posts(current_posts);

			FFS::FS::sync_inode_table();
		}

		open_files.erase(inode);
	}
}

FFS::inode_t FFS::FileHandle::inode(FFS::file_handle_t fh) {
	return (FFS::inode_t) fh;
}

FFS::inode_t FFS::FileHandle::parent(FFS::file_handle_t fh) {
	auto inode = FFS::FileHandle::inode(fh);
	return get_open_file(inode).parent();
}

void FFS::FileHandle::update_stream(FFS::file_handle_t fh, std::shared_ptr<std::iostream> stream) {
	auto inode = FFS::FileHandle::inode(fh);
	auto& open_file = get_open_file(inode);
	open_file.set_stream(stream);
	open_file.set_size(FFS::stream_size(*stream));
}

std::shared_ptr<std::iostream> FFS::FileHandle::get_stream(FFS::file_handle_t fh) {
	auto inode = FFS::FileHandle::inode(fh);
	auto& open_file = get_open_file(inode);
	
	return open_file.get_stream();
}

bool FFS::FileHandle::is_modified(FFS::file_handle_t fh) {
	auto inode = FFS::FileHandle::inode(fh);

	// Special case if it does not contain, simply has not been modified
	if(!open_files.contains(inode))
		return false;

	auto& open_file = get_open_file(inode);
	return open_file.is_modified();
}

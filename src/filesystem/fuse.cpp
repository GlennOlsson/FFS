#define FUSE_USE_VERSION 26

#include <fuse.h>

#include "fuse.h"
#include "fs.h"
#include "storage.h"
#include "inode_table.h"
#include "directory.h"

#include "../helpers/functions.h"

#include "../exceptions/exceptions.h"

#include <string>
#include <iostream>
#include <sstream>
#include <math.h>

// Read and write for everyone
#define PERM_OWNER (S_IRWXU)
#define PERM_GROUP (S_IRWXG)
#define PERM_OTHER (S_IRWXO)
#define FILE_PERMISSIONS ( PERM_OWNER | PERM_GROUP | PERM_OTHER )

static int ffs_getattr(const char* path, struct stat* stat_struct) {
	if(!FFS::FS::exists(path))
		return -ENOENT;

	auto entry = FFS::FS::entry(path);
	if(entry->is_dir) {
		auto blobs = FFS::Storage::get_file(entry->post_blocks);
		auto dir = FFS::Storage::dir_from_blobs(blobs);

		stat_struct->st_mode = S_IFDIR | FILE_PERMISSIONS;
		stat_struct->st_nlink = 2 + dir->entries->size(); // ., .. and all entries
	} else {
		stat_struct->st_mode = S_IFREG | FILE_PERMISSIONS;
		stat_struct->st_nlink = 1;
		stat_struct->st_size = entry->length;
	}

	return 0;
}

static int ffs_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
	auto dir = FFS::FS::read_dir(path);
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	for(auto entry: *dir->entries) {
		filler(buf, entry.first.c_str(), NULL, 0);
	}

	return 0;
}

static int ffs_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
	if(!FFS::FS::exists(path)) 
		return -ENOENT;
	
	std::stringbuf string_buf;
	std::basic_iostream stream(&string_buf);
	FFS::FS::read_file(path, stream);
	
	stream.seekg(offset);
	int index = 0;
	// Read as many bytes as requested, or until end of stream
	while(index < size && !stream.eof()) {
		FFS::read_c(stream, buf[index++]);
	}

	// Return either the amount of bytes requested, or the amount read if its lower
	return std::min((int) size, index);
}

static int ffs_write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {	
	if(!FFS::FS::exists(path)) 
		return -ENOENT;
	
	if(FFS::FS::is_dir(path))
		return -EISDIR;

	// Create stream for new file content
	std::stringbuf new_string_buf;
	std::basic_iostream new_stream(&new_string_buf);

	// If offset > 0, read current file and add up until offset before
	if(offset > 0) {
		std::stringbuf curr_string_buf;
		std::basic_iostream curr_stream(&curr_string_buf);

		FFS::FS::read_file(path, curr_stream);
		
		int i = 0;
		while(i++ < offset) {
			new_stream.put(curr_stream.get());
		}
	}

	// Add new content
	size_t index = 0;
	while(index < size) {
		FFS::write_c(new_stream, buf[index++]);
	}

	FFS::FS::remove(path);
	FFS::FS::create_file(path, new_stream);

	return size;
}

int ffs_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
	if(FFS::FS::exists(path))
		return -EACCES;
	
	std::stringbuf buf;
	std::istream empty_stream(&buf);

	FFS::FS::create_file(path, empty_stream);

	return 0;
}

int ffs_mkdir(const char* path, mode_t mode) {
	if(FFS::FS::exists(path))
		return -EACCES;
	
	FFS::FS::create_dir(path);

	return 0;
}

static int ffs_unlink(const char * path) {
	if(!FFS::FS::exists(path))
		return -ENOENT;
	
	try {
		FFS::FS::remove(path);
	} catch (FFS::Exception) {
		return 1;
	}

	return 0;
}

static int ffs_rmdir(const char * path) {
	if(!FFS::FS::exists(path))
		return -ENOENT;
	
	if(!FFS::FS::is_dir(path))
		return -ENOTDIR;

	try {
		FFS::FS::remove(path);
	} catch (FFS::Exception) {
		return 1;
	}

	return 0;
}

static int ffs_rename(const char* from, const char* to_c) {
	std::cerr << "Move from \"" << std::string(from) << "\"" <<std::endl;
	std::cerr << "To \"" << std::string(to_c) << "\"" << std::endl;

	if(!FFS::FS::exists(from))
		return -ENOENT;

	std::string to(to_c);
	auto filename_to = FFS::FS::filename(to);
	// remove filename, and the / before

	std::cerr << "filename to: \"" << filename_to << "\"" << std::endl;

	// Remove /filename from to_path, as we need to make sure the path before exists
	auto offset = to.rfind(filename_to);
	std::cerr << "Offset in to path: " << offset << std::endl;
	if(offset == std::string::npos || offset < 1)
		throw FFS::NoPathWithName(to);
	auto to_parent = to.substr(0, offset - 1);

	std::cerr << "to_parent path: \"" << to_parent << "\"" << std::endl;

	if(!FFS::FS::exists(to_parent))
		return -ENOENT;


	auto parent_from = FFS::FS::parent_entry(from);
	auto filename_from = FFS::FS::filename(from);

	auto inode = parent_from.second->remove_entry(filename_from);

	auto parent_to = FFS::FS::parent_entry(to);
	parent_to.second->add_entry(filename_to, inode);

	FFS::Storage::update(*parent_from.second, parent_from.first);
	FFS::Storage::update(*parent_to.second, parent_to.first);

	return 0;
}

static struct fuse_operations ffs_operations = {
	.getattr	= ffs_getattr,
	.read		= ffs_read,   
	.readdir	= ffs_readdir,
	.write		= ffs_write,
	.create		= ffs_create,
	.mkdir		= ffs_mkdir,
	.unlink		= ffs_unlink,
	.rmdir		= ffs_rmdir,
	.rename		= ffs_rename,
};

int FFS::FUSE::start(int argc, char *argv[]) {
	std::cout << "+ ------------------------ +" << std::endl;
	std::cout << "|                          |" << std::endl;
	std::cout << "|  Welcome to FFS! You can |" << std::endl;
	std::cout << "|  find the filesystem in  |" << std::endl;
	std::cout << "|   WORKING_DIR/ffs or in  |" << std::endl;
	std::cout << "|   the volumes directory  |" << std::endl;
	std::cout << "|                          |" << std::endl;
	std::cout << "+ ------------------------ +" << std::endl;
	return fuse_main(argc, argv, &ffs_operations, NULL);
}
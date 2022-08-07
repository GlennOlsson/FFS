#define FUSE_USE_VERSION 26

#include <fuse.h>

#include "fuse.h"
#include "fs.h"
#include "storage.h"
#include "inode_table.h"
#include "directory.h"
#include "file_handle.h"
#include "file_coder.h"

#include "../system/state.h"

#include "../helpers/functions.h"
#include "../helpers/constants.h"

#include "../exceptions/exceptions.h"

#include <string>
#include <iostream>
#include <sstream>
#include <math.h>
#include <sys/unistd.h>

// Read and write for everyone
#define PERM_OWNER (S_IRWXU)
#define PERM_GROUP (S_IRWXG)
#define PERM_OTHER (S_IRWXO)
#define FULL_PERMISSIONS ( PERM_OWNER | PERM_GROUP | PERM_OTHER )

std::size_t _replace_all(std::string& inout, std::string_view what, std::string_view with) {
	std::size_t count{};
	for (std::string::size_type pos{};
		 inout.npos != (pos = inout.find(what.data(), pos, what.length()));
		 pos += with.length(), ++count) {
		inout.replace(pos, what.length(), with.data(), with.length());
	}
	return count;
}

std::string sanitize_path(const char* c_path) {
	std::string str(c_path);
	_replace_all(str, "\\ ", " ");
	_replace_all(str, "ö", "ö"); // force replace special ö inserted my macos

	return str;
}

static int ffs_getattr(const char* c_path, struct stat* stat_struct) {
	auto path = sanitize_path(c_path);
	std::cout << "ffs_getattr " << path << std::endl;

	if(!FFS::FS::exists(path))
		return -ENOENT;

	auto entry = FFS::FS::entry(path);
	if(entry->is_dir) {
		auto dir = FFS::FS::get_dir(entry);

		stat_struct->st_mode = S_IFDIR | FULL_PERMISSIONS;
		stat_struct->st_nlink = 2 + dir->entries->size(); // ., .. and all entries
	} else {
		stat_struct->st_mode = S_IFREG | FULL_PERMISSIONS;

		stat_struct->st_nlink = 1;
		stat_struct->st_size = entry->length;

		stat_struct->st_birthtimespec.tv_nsec = entry->time_created;
		stat_struct->st_birthtimespec.tv_sec = entry->time_created / 1000;

		stat_struct->st_atimespec.tv_nsec = entry->time_accessed;
		stat_struct->st_atimespec.tv_sec = entry->time_accessed / 1000;

		stat_struct->st_mtimespec.tv_nsec = entry->time_modified;
		stat_struct->st_mtimespec.tv_sec = entry->time_modified / 1000;
	}

	std::cout << "End of ffs_getattr " << path << std::endl << std::endl;
	return 0;
}

static int ffs_fgetattr(const char* c_path, struct stat* stat_struct, struct fuse_file_info* fi) {
	return ffs_getattr(c_path, stat_struct);
}

static int ffs_readdir(const char* c_path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
	std::cout << "ffs_readir " << c_path << std::endl;

	auto inode = FFS::FileHandle::inode(fi->fh);
	auto dir = FFS::FS::get_dir(inode);

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	for(auto entry: *dir->entries) {
		filler(buf, entry.first.c_str(), NULL, 0);
	}

	std::cout << "End of ffs_readir " << c_path << std::endl << std::endl;
	return 0;
}

static int ffs_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
	std::cout << "read " << path << ", size: " << size << ", offset: " << offset << std::endl;

	auto inode = FFS::FileHandle::inode(fi->fh);
	
	std::stringbuf string_buf;
	std::basic_iostream stream(&string_buf);
	FFS::FS::read_file(inode, stream);
	
	stream.seekg(offset);
	int index = 0;
	// Read as many bytes as requested, or until end of stream
	while(index < size && !stream.eof()) {
		FFS::read_c(stream, buf[index++]);
	}

	std::cout << "End of read " << path << std::endl << std::endl;
	// Return either the amount of bytes requested, or the amount read if its lower
	return std::min((int) size, index);
}

static int ffs_write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
	std::cout << "write " << path << std::endl;

	auto fh = fi->fh;
	auto inode = FFS::FileHandle::inode(fh);

	// Create stream for new file content
	std::stringbuf new_string_buf;
	std::basic_iostream new_stream(&new_string_buf);

	// If offset > 0, read current file and add up until offset before
	if(offset > 0) {
		std::stringbuf curr_string_buf;
		std::basic_iostream curr_stream(&curr_string_buf);

		// If the file has been modified, i.e. is storing new blobs, read those as current file instead
		if(FFS::FileHandle::is_modified(fh)) {
			auto curr_blobs = FFS::FileHandle::get_blobs(fh);
			FFS::decode(curr_blobs, curr_stream);
		} else
			FFS::FS::read_file(inode, curr_stream);
		
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

    new_stream.flush();

	auto new_blobs = FFS::FS::update_file(inode, new_stream);
	FFS::FileHandle::update_blobs(fh, new_blobs);

	std::cout << "End of write " << path << std::endl << std::endl;
	return size;
}

int ffs_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
	std::cout << "create " << path << std::endl;
	
	// Create empty file and update parent dir in storage medium
	auto fh = FFS::FileHandle::create(path);
	fi->fh = fh;

	std::cout << "End of create " << path << std::endl << std::endl;
	return 0;
}

int ffs_mkdir(const char* c_path, mode_t mode) {
	auto path = sanitize_path(c_path);
	std::cout << "mkdir " << path << std::endl;
	
	FFS::FS::create_dir(path);

	FFS::FS::sync_inode_table();
	
	std::cout << "End of mkdir " << path << std::endl << std::endl;
	return 0;
}

static int ffs_unlink(const char * c_path) {
	auto path = sanitize_path(c_path);
	std::cout << "unlink " << path << std::endl;
	
	try {
		FFS::FS::remove(path);
	} catch (FFS::Exception) {
		return 1;
	}

	FFS::FS::sync_inode_table();
	
	std::cout << "End of unlink " << path << std::endl << std::endl;
	return 0;
}

static int ffs_rmdir(const char * c_path) {
	auto path = sanitize_path(c_path);
	std::cout << "rmdir " << path << std::endl;


	try {
		FFS::FS::remove(path);
	} catch (FFS::Exception) {
		return 1;
	}

	FFS::FS::sync_inode_table();
	
	std::cout << "End of rmdir " << path << std::endl << std::endl;
	return 0;
}

static int ffs_rename(const char* c_from, const char* c_to) {
	auto from = sanitize_path(c_from);

	auto to = sanitize_path(c_to);
	auto filename_to = FFS::FS::filename(to);

	std::cout << "rename " << from << ", " << to << std::endl;

	// Remove /filename from to_path, as we need to make sure the path before exists
	auto offset = to.rfind(filename_to);
	if(offset == std::string::npos || offset < 1)
		throw FFS::NoPathWithName(to);
	auto to_parent = to.substr(0, offset);

	auto parent_from = FFS::FS::parent_entry(from);
	auto filename_from = FFS::FS::filename(from);

	auto inode = parent_from->second->remove_entry(filename_from);

	auto parent_to = FFS::FS::parent_entry(to);
	parent_to->second->add_entry(filename_to, inode);

	// If the parent is same, only update once
	FFS::Storage::update(parent_from->second, parent_from->first);
	if(parent_to->second != parent_to->second)
		FFS::Storage::update(parent_to->second, parent_to->first);

	FFS::FS::sync_inode_table();
	
	std::cout << "End of rename " << from << ", " << to << std::endl << std::endl;
	return 0;
}

// truncate or extend file to be size bytes
static int ffs_truncate(const char* c_path, off_t size) {
	auto path = sanitize_path(c_path);
	std::cout << "truncate " << path << std::endl;

	if(!FFS::FS::exists(path)) 
		return -ENOENT;
	
	if(FFS::FS::is_dir(path))
		return -EISDIR;

	std::stringbuf new_string_buf;
	std::basic_iostream new_stream(&new_string_buf);

	std::stringbuf curr_string_buf;
	std::basic_iostream curr_stream(&curr_string_buf);

	FFS::FS::read_file(path, curr_stream);
	
	// While there is content in the stream and the index 
	// 	is smaller than the size, add from current stream
	int new_size = 0;
	while(curr_stream && new_size++ < size) {
		new_stream.put(curr_stream.get());
	}

	// If extending the file, continue to add NULL to match the new size of the file
	while(new_size++ < size) {
		new_stream.put('\0');
	}

	FFS::FS::remove(path);

	auto ptr = std::make_shared<std::istream>(new_stream.rdbuf());
	FFS::FS::create_file(path, ptr);

	FFS::FS::sync_inode_table();
	
	std::cout << "End of truncate " << path << std::endl << std::endl;
	return size;
}

// Similar to above but uses file handle
static int ffs_ftruncate(const char* path, off_t size, fuse_file_info* fi) {
	std::cout << "Start of ftruncate " << path << std::endl << std::endl;
	
	auto fh = fi->fh;
	auto inode = FFS::FileHandle::inode(fh);

	std::stringbuf new_string_buf;
	std::basic_iostream new_stream(&new_string_buf);

	std::stringbuf curr_string_buf;
	std::basic_iostream curr_stream(&curr_string_buf);

	if(FFS::FileHandle::is_modified(fh)) {
		auto curr_blobs = FFS::FileHandle::get_blobs(fh);
		FFS::decode(curr_blobs, curr_stream);
	} else
		FFS::FS::read_file(inode, curr_stream);

	// While there is content in the stream and the index 
	// 	is smaller than the size, add from current stream
	int new_size = 0;
	while(curr_stream && new_size++ < size) {
		new_stream.put(curr_stream.get());
	}

	// If extending the file, continue to add NULL to match the new size of the file
	while(new_size++ < size) {
		new_stream.put('\0');
	}

	auto new_blobs = FFS::FS::update_file(inode, new_stream);
	FFS::FileHandle::update_blobs(fh, new_blobs);

	std::cout << "End of ftruncate " << path << std::endl << std::endl;

	return 0;
}

/*
	unsigned long	f_bsize;	// File system block size
	unsigned long	f_frsize;	// Fundamental file system block size
	fsblkcnt_t		f_blocks;	// Blocks on FS in units of f_frsize
	fsblkcnt_t		f_bfree;	// Free blocks
	fsblkcnt_t		f_bavail;	// Blocks available to non-root
	fsfilcnt_t		f_files;	// Total inodes
	fsfilcnt_t		f_ffree;	// Free inodes
	fsfilcnt_t		f_favail;	// Free inodes for non-root
	unsigned long	f_fsid;		// Filesystem ID
	unsigned long	f_flag;		// Bit mask of values
	unsigned long	f_namemax;	// Max filename length
*/

static int ffs_statfs(const char* path, struct statvfs* stbuf) {
	// (Max) block size
	stbuf->f_bsize = FFS_MAX_FILE_SIZE;

	stbuf->f_frsize = FFS_MAX_FILE_SIZE;

	stbuf->f_blocks = 1000;

	// Always 1000 blocks available (infinite storage o_O)
	stbuf->f_bfree = 1000;

	// Many free inodes, always
	stbuf->f_ffree = 1000000;

	// Filesystem ID, something with FFS
	stbuf->f_fsid = (('F' < 5) | 'F') | 'S';

	// Filename can be up to 128 bytes (1028 bits)
	stbuf->f_namemax = 128;

	return 0;
}

static int ffs_access(const char* c_path, int mask) {
	auto path = sanitize_path(c_path);
	if(!FFS::FS::exists(path))
		return -ENOENT;

	return F_OK;
}

static int ffs_utimens(const char* c_path, const struct timespec ts[2]) {
	auto path = sanitize_path(c_path);
	if(!FFS::FS::exists(path))
		return -ENOENT;
	
	auto entry = FFS::FS::entry(path);

	auto ms_since_epoch = FFS::curr_milliseconds();

	auto new_access_time = ts[0].tv_nsec;
	auto new_modified_time = ts[1].tv_nsec;

	bool change_made = false;

	// Only update if value is greater than last time, but smaller than current time
	if((new_access_time > entry->time_accessed) && (new_access_time <= ms_since_epoch)) {
		entry->time_accessed = new_access_time;
		change_made = true;
	}
	
	if((new_modified_time > entry->time_modified) && (new_modified_time <= ms_since_epoch))	 {
		entry->time_modified = new_modified_time;
		change_made = true;
	}

	if(change_made)
		FFS::FS::sync_inode_table();
		
	return 0;
}

static int ffs_opendir(const char* path, struct fuse_file_info* fi) {
	std::cout << "open dir " << path << std::endl;
	auto fh = FFS::FileHandle::open(path);
	fi->fh = fh;
	std::cout << "end of open dir" << std::endl;
	return 0;
}


static int ffs_open(const char* path, struct fuse_file_info* fi) {
	std::cout << "open file " << path << std::endl;
	auto fh = FFS::FileHandle::open(path);
	fi->fh = fh;
	std::cout << "end of open file" << std::endl;
	return 0;
}

static int ffs_releasedir(const char* path, struct fuse_file_info *fi) {
	std::cout << "close dir " << path << std::endl;
	FFS::FileHandle::close(fi->fh);
	std::cout << "end of close dir" << std::endl;
	return 0;
}

static int ffs_release(const char* path, struct fuse_file_info *fi) {
	std::cout << "close file " << path << std::endl;
	FFS::FileHandle::close(fi->fh);
	std::cout << "end of close file" << std::endl;
	return 0;
}

// --- UNIMPLEMENTED ---
static int ffs_flush(const char* path, struct fuse_file_info* fi) {
	// Do nothing interesting, but don't return error
	return 0;
}

static int ffs_readlink(const char* path, char* buf, size_t size) {
	std::cerr << "UNIMPLEMENTED: readlink" << std::endl;
	return -EPERM;
}

static int ffs_symlink(const char* to, const char* from) {
	std::cerr << "UNIMPLEMENTED: symlink" << std::endl;
	return -EPERM;
}

static int ffs_link(const char* from, const char* to) {
	std::cerr << "UNIMPLEMENTED: link" << std::endl;
	return -EPERM;
}

static int ffs_chmod(const char* path, mode_t mode) {
	std::cerr << "UNIMPLEMENTED: chmod" << std::endl;
	return -EPERM;
}

static int ffs_chown(const char* path, uid_t uid, gid_t gid) {
	std::cerr << "UNIMPLEMENTED: chown" << std::endl;
	return -EPERM;
}

static int ffs_fsync(const char* path, int isdatasync, struct fuse_file_info* fi) {
	std::cerr << "UNIMPLEMENTED: fsync" << std::endl;
	return -EPERM;
}

static int ffs_fsyncdir(const char* path, int isdatasync, struct fuse_file_info* fi) {
	std::cerr << "UNIMPLEMENTED: fsyncdir" << std::endl;
	return -EPERM;
}


//static int ffs_lock(const char* path, struct fuse_file_info* fi, int cmd, struct flock* locks) {
//	std::cerr << "UNIMPLEMENTED: lock" << std::endl;
//	return -EPERM;
//}

static int ffs_bmap(const char* path, size_t blocksize, uint64_t* blockno) {
	std::cerr << "UNIMPLEMENTED: bmap" << std::endl;
	return -EPERM;
}

//static int ffs_setxattr(const char* path, const char* name, const char* value, size_t size, int flags, uint32_t unknown) {
//	std::cerr << "UNIMPLEMENTED: setxattr" << std::endl;
//	return -EPERM;
//}

//static int ffs_getxattr(const char* path, const char* name, char* value, size_t size, uint32_t unknown) {
//	std::cerr << "UNIMPLEMENTED: getxattr" << std::endl;
//	return -EPERM;
//}

//static int ffs_listxattr(const char* path, char* list, size_t size) {
//	std::cerr << "UNIMPLEMENTED: listxattr" << std::endl;
//	return -EPERM;
//}

static int ffs_ioctl(const char* path, int cmd, void* arg, struct fuse_file_info* fi, unsigned int flags, void* data) {
	std::cerr << "UNIMPLEMENTED: ioctl" << std::endl;
	return -EPERM;
}

static int ffs_poll(const char* path, struct fuse_file_info* fi, struct fuse_pollhandle* ph, unsigned* reventsp) {
	std::cerr << "UNIMPLEMENTED: poll" << std::endl;
	return -EPERM;
}


static struct fuse_operations ffs_operations = {
	.getattr	= ffs_getattr,
	.fgetattr	= ffs_fgetattr,
	.read		= ffs_read,   
	.readdir	= ffs_readdir,
	.write		= ffs_write,
	.create		= ffs_create,
	.mkdir		= ffs_mkdir,
	.unlink		= ffs_unlink,
	.rmdir		= ffs_rmdir,
	.rename		= ffs_rename,
	.truncate	= ffs_truncate,
	.ftruncate	= ffs_ftruncate,
	.statfs		= ffs_statfs,
	.access		= ffs_access,
	.utimens	= ffs_utimens,
	.open		= ffs_open,
	.release	= ffs_release,
	.opendir	= ffs_opendir,
	.releasedir	= ffs_releasedir,

	// -- UNIMPLEMENTED -- 
	.flush		= ffs_flush,
	.readlink	= ffs_readlink,
	.symlink	= ffs_symlink,
	.link		= ffs_link,
	.chmod		= ffs_chmod,
	.chown		= ffs_chown,
	.fsync		= ffs_fsync,
	.fsyncdir	= ffs_fsyncdir,
	//.lock		= ffs_lock,
	.bmap		= ffs_bmap,
	//.setxattr	= ffs_setxattr,
	//.getxattr	= ffs_getxattr,
	//.listxattr	= ffs_listxattr,
	.ioctl		= ffs_ioctl,
	.poll		= ffs_poll,
};

int FFS::FUSE::start(int argc, char *argv[]) {
	std::cout << "+ ------------------------ +" << std::endl;
	std::cout << "|                          |" << std::endl;
	std::cout << "|  Welcome to FFS! You can |" << std::endl;
	std::cout << "|    find the filesystem   |" << std::endl;
	std::cout << "|   under /ffs/ or in the  |" << std::endl;
	std::cout << "|    volumes directory     |" << std::endl;
	std::cout << "|                          |" << std::endl;
	std::cout << "+ ------------------------ +" << std::endl;
	
	auto fuse_ret = fuse_main(argc, argv, &ffs_operations, NULL);

	std::cout << "+ ----------------------- +" << std::endl;
	std::cout << "|                         |" << std::endl;
	std::cout << "|         FFS is          |" << std::endl;
	std::cout << "|    unmounting safely.   |" << std::endl;
	std::cout << "|                         |" << std::endl;
	std::cout << "|     Do NOT force the    |" << std::endl;
	std::cout << "|   program to shut down  |" << std::endl;
	std::cout << "|                         |" << std::endl;
	std::cout << "+ ----------------------- +" << std::endl;

	FFS::FS::sync_inode_table();



	return fuse_ret;
}
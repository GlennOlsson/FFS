#define FUSE_USE_VERSION 26

#include <fuse.h>

#include "fuse.h"
#include "fs.h"
#include "storage.h"
#include "inode_table.h"
#include "directory.h"

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
#define FILE_PERMISSIONS ( PERM_OWNER | PERM_GROUP | PERM_OTHER )

std::size_t replace_all(std::string& inout, std::string_view what, std::string_view with) {
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
	replace_all(str, "\\ ", " ");

	return str;
}

static int ffs_getattr(const char* c_path, struct stat* stat_struct) {
	auto path = sanitize_path(c_path);

	if(!FFS::FS::exists(path))
		return std::cerr << "PATH NOT EXISTS \"" << path << "\"" << std::endl, -ENOENT;

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

static int ffs_fgetattr(const char* c_path, struct stat* stat_struct, struct fuse_file_info* fi) {
	return ffs_getattr(c_path, stat_struct);
}

static int ffs_readdir(const char* c_path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
	auto path = sanitize_path(c_path);
	
	std::cout << "read dir " << path << std::endl;

	auto dir = FFS::FS::read_dir(path);
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	for(auto entry: *dir->entries) {
		filler(buf, entry.first.c_str(), NULL, 0);
	}

	return 0;
}

static int ffs_read(const char* c_path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
	auto path = sanitize_path(c_path);
	
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

static int ffs_write(const char* c_path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
	auto path = sanitize_path(c_path);

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

int ffs_create(const char* c_path, mode_t mode, struct fuse_file_info* fi) {
	auto path = sanitize_path(c_path);
	
	if(FFS::FS::exists(path))
		return -EACCES;
	
	std::stringbuf buf;
	std::istream empty_stream(&buf);

	FFS::FS::create_file(path, empty_stream);

	return 0;
}

int ffs_mkdir(const char* c_path, mode_t mode) {
	auto path = sanitize_path(c_path);
	std::cerr << "mkdir " << path << std::endl; 
	
	if(FFS::FS::exists(path))
		return -EACCES;
	
	FFS::FS::create_dir(path);
	std::cerr << "created " << path << std::endl;

	return 0;
}

static int ffs_unlink(const char * c_path) {
	auto path = sanitize_path(c_path);
	
	if(!FFS::FS::exists(path))
		return -ENOENT;
	
	try {
		FFS::FS::remove(path);
	} catch (FFS::Exception) {
		return 1;
	}

	return 0;
}

static int ffs_rmdir(const char * c_path) {
	auto path = sanitize_path(c_path);
	
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

static int ffs_rename(const char* c_from, const char* c_to) {
	auto from = sanitize_path(c_from);
	std::cerr << "rename from " << c_from << std::endl;

	if(!FFS::FS::exists(from))
		return std::cerr << "rename from does not exist " << from << std::endl, -ENOENT;

	auto to = sanitize_path(c_to);
	auto filename_to = FFS::FS::filename(to);
	std::cerr << "rename to " << to << ", filname " << filename_to << std::endl;
	// remove filename, and the / before

	// Remove /filename from to_path, as we need to make sure the path before exists
	auto offset = to.rfind(filename_to);
	if(offset == std::string::npos || offset < 1)
		throw FFS::NoPathWithName(to);
	auto to_parent = to.substr(0, offset);

	if(!FFS::FS::exists(to_parent))
		return std::cerr << "to parent does not exist " << to_parent << std::endl, -ENOENT;

	auto parent_from = FFS::FS::parent_entry(from);
	auto filename_from = FFS::FS::filename(from);

	auto inode = parent_from->second->remove_entry(filename_from);

	auto parent_to = FFS::FS::parent_entry(to);
	parent_to->second->add_entry(filename_to, inode);

	FFS::Storage::update(*parent_from->second, parent_from->first);
	// Only need to update the dir once if they are the same
	if(parent_from->first != parent_to->first)
		FFS::Storage::update(*parent_to->second, parent_to->first);

	std::cerr << "renamed " << from << " to " << to << std::endl;

	return 0;
}

static int ffs_truncate(const char* c_path, off_t size) {
	auto path = sanitize_path(c_path);

	if(!FFS::FS::exists(path)) 
		return -ENOENT;
	
	if(FFS::FS::is_dir(path))
		return -EISDIR;

	std::stringbuf new_string_buf;
	std::basic_iostream new_stream(&new_string_buf);

	std::stringbuf curr_string_buf;
	std::basic_iostream curr_stream(&curr_string_buf);

	FFS::FS::read_file(path, curr_stream);
	
	int i = 0;
	while(i++ < size) {
		new_stream.put(curr_stream.get());
	}

	FFS::FS::remove(path);
	FFS::FS::create_file(path, new_stream);

	return size;
}

// Same as above but called by user program
static int ffs_ftruncate(const char* path, off_t size, fuse_file_info* fi) {
	return ffs_truncate(path, size);
}

/*
	unsigned long	f_bsize;	// File system block size
	unsigned long	f_frsize;	// Fundamental file system block size
	fsblkcnt_t	f_blocks;	// Blocks on FS in units of f_frsize
	fsblkcnt_t	f_bfree;	// Free blocks
	fsblkcnt_t	f_bavail;	// Blocks available to non-root
	fsfilcnt_t	f_files;	// Total inodes
	fsfilcnt_t	f_ffree;	// Free inodes
	fsfilcnt_t	f_favail;	// Free inodes for non-root
	unsigned long	f_fsid;		// Filesystem ID
	unsigned long	f_flag;		// Bit mask of values
	unsigned long	f_namemax;	// Max file name length
*/

static int ffs_statfs(const char* path, struct statvfs* stbuf) {
	// Filename can be up to 128 bytes (1028 bits)
	stbuf->f_namemax = 128;
	
	// Filesystem ID, something with FFS
	stbuf->f_fsid = (('F' < 5) | 'F') | 'S';

	// (Max) block size
	stbuf->f_bsize = FFS_MAX_FILE_SIZE;

	return 0;
}

static int ffs_access(const char* path, int mask) {
	if(!FFS::FS::exists(path))
		return -ENOENT;

	return F_OK;
}

static int ffs_flush(const char* path, struct fuse_file_info* fi) {
	// Do nothing interesting, but don't return error
	return 0;
}

// --- UNIMPLEMENTED ---

static int ffs_readlink(const char* path, char* buf, size_t size) {
	std::cerr << "UNIMPLEMENTED: readlink" << std::endl;
	return -EPERM;
}

//static int ffs_opendir(const char* path, struct fuse_file_info* fi) {
//	std::cerr << "UNIMPLEMENTED: opendir" << std::endl;
//	return -EPERM;
//}

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

static int ffs_utimens(const char* path, const struct timespec ts[2]) {
	std::cerr << "UNIMPLEMENTED: utimens" << std::endl;
	return -EPERM;
}

//static int ffs_open(const char* path, struct fuse_file_info* fi) {
//	std::cerr << "UNIMPLEMENTED: open" << std::endl;
//	return -EPERM;
//}

//static int ffs_release(const char* path, struct fuse_file_info *fi) {
//	std::cerr << "UNIMPLEMENTED: release" << std::endl;
//	return -EPERM;
//}

//static int ffs_releasedir(const char* path, struct fuse_file_info *fi) {
//	std::cerr << "UNIMPLEMENTED: releasedir" << std::endl;
//	return -EPERM;
//}

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

	// -- UNIMPLEMENTED -- 
	.readlink	= ffs_readlink,
	//.opendir	= ffs_opendir,
	.symlink	= ffs_symlink,
	.link		= ffs_link,
	.chmod		= ffs_chmod,
	.chown		= ffs_chown,
	.utimens	= ffs_utimens,
	//.open		= ffs_open,
	//.release	= ffs_release,
	//.releasedir	= ffs_releasedir,
	.fsync		= ffs_fsync,
	.fsyncdir	= ffs_fsyncdir,
	.flush		= ffs_flush,
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
	return fuse_main(argc, argv, &ffs_operations, NULL);
}
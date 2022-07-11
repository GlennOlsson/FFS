#define FUSE_USE_VERSION 26

#include <fuse.h>

#include "fuse.h"
#include "fs.h"
#include "storage.h"
#include "inode_table.h"
#include "directory.h"

#include "../helpers/functions.h"

#include <string>
#include <iostream>
#include <sstream>
#include <math.h>

// Read and write for everyone
#define PERM_OWNER (S_IRUSR | S_IWUSR)
#define PERM_GROUP (S_IRGRP | S_IWGRP)
#define PERM_OTHER ( S_IROTH | S_IWOTH)
#define FILE_PERMISSIONS ( PERM_OWNER | PERM_GROUP | PERM_OTHER )

static int ffs_getattr(const char* path, struct stat* stat_struct) {
    if(!FFS::FS::exists(path))
        return -ENOENT;

    FFS::InodeEntry* entry = FFS::FS::entry(path);
    if(entry->is_dir) {
        auto blobs = FFS::Storage::get_file(entry->post_blocks);
        FFS::Directory* dir = FFS::Storage::dir_from_blobs(blobs);

        stat_struct->st_mode = S_IFDIR | 0755;
        stat_struct->st_nlink = 2 + dir->entries->size(); // ., .. and all entries
    } else {
        stat_struct->st_mode = S_IFREG | FILE_PERMISSIONS;
        stat_struct->st_nlink = 1;
        stat_struct->st_size = entry->length;
    }

    return 0;
}

static int ffs_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
    FFS::Directory* dir = FFS::FS::read_dir(path);
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

static struct fuse_operations ffs_operations = {
    .getattr = ffs_getattr, /* To provide size, permissions, etc. */
    // .open    = hello_open,    /* To enforce read-only access.       */
    .read    = ffs_read,    /* To provide file content.           */
    .readdir = ffs_readdir, /* To provide directory listing.      */
};

int FFS::FUSE::start(int argc, char *argv[]) {
     return fuse_main(argc, argv, &ffs_operations, NULL);
}
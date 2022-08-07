#ifndef FFS_FS_H
#define FFS_FS_H

#include "directory.h"
#include "inode_table.h"

#include <string>
#include <istream>
#include <memory>

namespace FFS::FS {
    struct Traverser {
        std::shared_ptr<FFS::Directory> parent_dir;
        FFS::inode_t parent_inode;
        std::string filename;

        std::string full_path;
    };

    // Traverse path and return Traverser struct
    std::shared_ptr<Traverser> traverse_path(std::string);
    // Verify filename of traverser is in directory
    void verify_in(std::shared_ptr<Traverser>);
    // Verify filename of traverser is NOT in directory
    void verify_not_in(std::shared_ptr<Traverser>);

    // Return the directory at path. Throws NoFileWithName if path does not exist
    std::shared_ptr<FFS::Directory> read_dir(std::string);
    // Return the directory with specified inode entry
    std::shared_ptr<FFS::Directory> get_dir(std::shared_ptr<FFS::InodeEntry>);
    // Return the directory with specified inode
    std::shared_ptr<FFS::Directory> get_dir(FFS::inode_t inode);

    // Writes to the stream with the content of the file with inode. Throws NoFileWithInode if file does not exist
    void read_file(FFS::inode_t inode, std::ostream& stream);
    // Writes to the stream with the content of the file at path. Throws NoFileWithName if path does not exist
    void read_file(std::string, std::ostream&);

    // Get the inode and directory entity of the parent directory of a file or directory
    std::shared_ptr<std::pair<FFS::inode_t, std::shared_ptr<FFS::Directory>>> parent_entry(std::string);

    // Get the filename of a path
    std::string filename(std::string);

    // Create an empty directory on path. Throws NoFileWithName if path up until directory does not exist
    void create_dir(std::string);
    
    // Update existing file with new content. Does not upload new blobs
    FFS::blobs_t update_file(FFS::inode_t, std::istream&);
    // Create a file at path, with content of stream. Throws NoFileWithName if path up until file does not exist
    void create_file(std::string, std::shared_ptr<std::istream>);

    // Remove file or directory at path. Throws NoFileWithName if path does not exist
    void remove(std::string, bool multithread = true);

    // Does the given path point to an existing file
    bool exists(std::string);

    // Is the entry at path a dir. If false it is a file. Throws NoFileWithName if path does not exist
    bool is_dir(std::string);

    // Get the inode entry from an inode
    std::shared_ptr<FFS::InodeEntry> entry(FFS::inode_t inode);
    // Get the inode entry from a path
    std::shared_ptr<FFS::InodeEntry> entry(std::string path);

    // Save the current state of the inode table
    void sync_inode_table();
}

#endif
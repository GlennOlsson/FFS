#ifndef FFS_FS_H
#define FFS_FS_H

#include "directory.h"
#include "inode_table.h"

#include <string>
#include <istream>
#include <memory>

namespace FFS::FS {
    // Return the directory at path. Throws NoFileWithName if path does not exist
    std::shared_ptr<FFS::Directory> read_dir(std::string);
    // Writes to the stream with the content of the file at path. Throws NoFileWithName if path does not exist
    void read_file(std::string, std::ostream&);

    // Get the inode entry of a file or directory
    std::shared_ptr<InodeEntry> entry(std::string);

    // Get the inode and directory entity of the parent directory of a file or directory
    std::shared_ptr<std::pair<FFS::inode_id, std::shared_ptr<FFS::Directory>>> parent_entry(std::string);

    // Get the filename of a path
    std::string filename(std::string);

    // Create an empty directory on path. Throws NoFileWithName if path up until directory does not exist
    void create_dir(std::string);
    
    // Create a file at path, with content of stream. Throws NoFileWithName if path up until file does not exist
    void create_file(std::string, std::istream&);

    // Remove file or directory at path. Throws NoFileWithName if path does not exist
    void remove(std::string);

    // Does the given path point to an existing file
    bool exists(std::string);

    // Is the entry at path a dir. If false it is a file. Throws NoFileWithName if path does not exist
    bool is_dir(std::string);
}

#endif
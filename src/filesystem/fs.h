#ifndef FFS_FS_H
#define FFS_FS_H

#include "directory.h"

#include <string>
#include <istream>

namespace FFS::FS {
    // Return the directory at path. Throws NoFileWithName if path does not exist
    FFS::Directory* read_dir(std::string);
    // Return a stream with the content of the file at path. Throws NoFileWithName if path does not exist
    std::istream* read_file(std::string);

    // Create an empty directory on path. Throws NoFileWithName if path up until directory does not exist
    void create_dir(std::string);
    
    // Create a file at path, with content of stream. Throws NoFileWithName if path up until file does not exist
    void create_file(std::string, std::istream*);

    // Remove file or directory at path. Throws NoFileWithName if path does not exist
    void remove(std::string);
}



#endif
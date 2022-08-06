#include "exceptions.h"

#include <string>
#include <sstream>

FFS::NoFileWithName::NoFileWithName(std::string name) :
    FFS::StorageException("There is no file named " + name)
{}

FFS::NoPathWithName::NoPathWithName(std::string name) :
    FFS::StorageException("There is no path named " + name)
{}

FFS::FileAlreadyExists::FileAlreadyExists(std::string name) :
    FFS::StorageException("There already exists a file or directory named " + name + ", cannot overwrite")
{}

FFS::NoFileWithInode::NoFileWithInode(FFS::inode_t i) : 
    FFS::StorageException("There is no file with inode " + std::to_string(i))
{};

FFS::BadFFSPath::BadFFSPath(std::string path, std::string bad_part) : 
    FFS::StorageException("The path [" + path + "] is bad due to [" + bad_part + "] not being a directory")
{};
#pragma once

#include "../helpers/types.h"

#include <string>
#include <stdexcept>

namespace FFS {

class Exception: public std::runtime_error {
protected:
	Exception(std::string reason) : std::runtime_error(reason) {}
};

class BadFFSFile: public FFS::Exception {
public:
	BadFFSFile(std::string reason) : Exception(reason) {}
};

class BadFFSHeader: public BadFFSFile {
public:
	BadFFSHeader(std::string why);
};

class UnexpectedEOF: public BadFFSFile {
public:
	UnexpectedEOF(uint64_t eof_location);
};

class StorageException: public FFS::Exception {
public:
	StorageException(std::string reason) : Exception(reason) {}
};

class NoFileWithName: public StorageException {
public:
	NoFileWithName(std::string file_name);
};

class FileAlreadyExists: public StorageException {
public:
	FileAlreadyExists(std::string file_name);
};

class NoFileWithInode: public StorageException {
public:
	NoFileWithInode(FFS::inode_id inode);
};

// when part of the path is a file rather than a directory
class BadFFSPath: public StorageException {
public:
	BadFFSPath(std::string path, std::string bad_part);
};

};
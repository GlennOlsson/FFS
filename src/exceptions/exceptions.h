#pragma once

#include "../helpers/types.h"

#include <string>

namespace FFS {
class BadFFSFile: public std::exception {
};

class BadFFSHeader: public BadFFSFile {
private:
	std::string reason_str;

public:
	BadFFSHeader(std::string why);
	const char* what() const noexcept;
};

class UnexpectedEOF: public BadFFSFile {
private:
	uint64_t location;

public:
	UnexpectedEOF(uint64_t eof_location);
	const char* what() const noexcept;
};

class StorageException: public std::exception {
};

class NoFileWithName: public StorageException {
private:
	std::string name;
public:
	NoFileWithName(std::string file_name);
	const char* what() const noexcept;
};

class FileAlreadyExists: public StorageException {
private:
	std::string name;
public:
	FileAlreadyExists(std::string file_name);
	const char* what() const noexcept;
};

class NoFileWithInode: public StorageException {
private:
	FFS::inode_id inode;
public:
	NoFileWithInode(FFS::inode_id inode);
	const char* what() const noexcept;
};

// when part of the path is a file rather than a directory
class BadFFSPath: public StorageException {
private:
	std::string path;
	std::string bad_part;
public:
	BadFFSPath(std::string path, std::string bad_part);
	const char* what() const noexcept;
};

};
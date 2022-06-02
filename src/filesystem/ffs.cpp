#include "ffs.h"

#include "directory.h"

#include <string>

int FFS::write(std::string ffs_path, char* src, size_t len) {
	return 1;
}

int FFS::read(std::string ffs_path, char* dest, size_t len) {
	return 1;
}

FFS::Directory* FFS::read_dir(std::string ffs_path) {
	return nullptr;
}

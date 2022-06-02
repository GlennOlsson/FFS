#ifndef FFS_H
#define FFS_H

#include "directory.h"

#include <string>

namespace FFS {
	int write(std::string ffs_path, char* src, size_t len);
	int read(std::string ffs_path, char* dest, size_t len);
	Directory* read_dir(std::string ffs_path);
}

#endif
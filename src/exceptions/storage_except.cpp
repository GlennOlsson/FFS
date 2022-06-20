#include "exceptions.h"

#include <string>
#include <sstream>

FFS::NoFileWithName::NoFileWithName(std::string name) {
    this->name = name;
}

const char* FFS::NoFileWithName::what() const noexcept {
    std::stringstream ss;
    ss << "There is no file named " << this->name;

    std::string str = ss.str();

	char* c_str = new char[str.size()];
	strcpy(c_str, str.c_str());

    return c_str;
}

FFS::NoFileWithInode::NoFileWithInode(FFS::inode_id i) : inode(i) {};

const char* FFS::NoFileWithInode::what() const noexcept {
std::stringstream ss;
    ss << "There is no file with inode if " << this->inode;

    std::string str = ss.str();

	char* c_str = new char[str.size()];
	strcpy(c_str, str.c_str());

    return c_str;
}
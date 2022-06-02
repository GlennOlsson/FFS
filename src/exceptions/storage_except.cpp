#include "exceptions.h"

#include <string>
#include <sstream>

FFS::NoFileWithName::NoFileWithName(std::string name, std::string parent_dir) {
    this->name = name;
    this->parent_dir = parent_dir;
}

const char* FFS::NoFileWithName::what() const noexcept {
    std::stringstream ss;
    ss << "There is no file named " << this->name << " in " << this->parent_dir;

    std::string str = ss.str();

	char* c_str = new char[str.size()];
	strcpy(c_str, str.c_str());

    return c_str;
}
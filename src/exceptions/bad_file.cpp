#include "exceptions.h"

#include <sstream>
#include <string>
#include <exception>


FFS::BadFFSHeader::BadFFSHeader(std::string why) {
	this->reason_str = why;
}

const char* FFS::BadFFSHeader::what() const noexcept {
	return this->reason_str.c_str();
}

FFS::UnexpectedEOF::UnexpectedEOF(uint64_t location) {
	this->location = location;
}

const char* FFS::UnexpectedEOF::what() const noexcept {
	std::stringstream ss;
	ss << "EOF at position " << this->location;

	std::string str = ss.str();

	char* c_str = new char[str.size()];
	strcpy(c_str, str.c_str());
	
	return c_str;
}
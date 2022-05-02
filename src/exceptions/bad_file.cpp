#include "exceptions.h"

#include <sstream>
#include <string>
#include <exception>


FFS::BadFFSHeader::BadFFSHeader(std::string why) {
	this->reason_str = why;
}

std::string FFS::BadFFSHeader::reason() {
	return this->reason_str;
}

FFS::EarlyEOF::EarlyEOF(uint64_t location) {
	this->location = location;
}

std::string FFS::EarlyEOF::reason() {
	std::stringstream ss;
	ss << "EOF at position " << this->location;
	return ss.str();
}
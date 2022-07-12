#include "exceptions.h"

#include <sstream>
#include <string>
#include <exception>


FFS::BadFFSHeader::BadFFSHeader(std::string why) : 
	FFS::BadFFSFile(why)
{}

FFS::UnexpectedEOF::UnexpectedEOF(uint64_t location) : 
	FFS::BadFFSFile("EOF at position " + std::to_string(location))
{}
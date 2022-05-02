#pragma once

#include <string>

namespace FFS {
class BadFile: public std::exception {
	virtual std::string reason() = 0;
};

class BadFFSHeader: public BadFile {
private:
	std::string reason_str;

public:
	BadFFSHeader(std::string why);
	std::string reason();
};

class EarlyEOF: public BadFile {
private:
	uint64_t location;

public:
	EarlyEOF(uint64_t eof_location);
	std::string reason();
};
};
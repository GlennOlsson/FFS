#pragma once

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
};
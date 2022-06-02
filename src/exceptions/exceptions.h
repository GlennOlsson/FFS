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

class StorageException: public std::exception {
};

class NoFileWithName: public StorageException {
private:
	std::string name;
	std::string parent_dir;
public:
	NoFileWithName(std::string file_name, std::string parent_dir);
	const char* what() const noexcept;
};

};
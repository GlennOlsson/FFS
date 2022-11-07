#include "functions.h"
#include "../exceptions/exceptions.h"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <sstream>

#include <chrono>

void FFS::set_random_seed(int v) { srand(v); }

uint32_t FFS::random_int() { return rand(); }

uint32_t FFS::random_int(uint32_t high) { return random_int() % high; }

uint32_t FFS::random_int(uint32_t low, uint32_t high) {
	return low + (random_int((high - low)));
}

uint64_t FFS::random_long() {
	return (static_cast<uint64_t>(random_int()) << (sizeof(int) * 8)) |
		   random_int();
}

uint64_t FFS::random_long(uint64_t high) { return random_long() % high; }

uint64_t FFS::random_long(uint64_t low, uint64_t high) {
	return low + (random_long((high - low)));
}

uint8_t FFS::random_byte() { return ((uint32_t)random_int()) % 255; }

std::string FFS::random_str(uint32_t length) {
	std::stringstream ss;
	while(length--) {
		ss << ((random_int() % 10) + 48);
	}
	return ss.str();
}

void FFS::write_c(std::ostream& stream, const uint8_t c) { stream.put(c); }
void FFS::write_i(std::ostream& stream, const uint32_t i) {
	stream.put((i >> (3 * 8)) & 0xFF);
	stream.put((i >> (2 * 8)) & 0xFF);
	stream.put((i >> (1 * 8)) & 0xFF);
	stream.put((i >> (0 * 8)) & 0xFF);
}
void FFS::write_l(std::ostream& stream, const uint64_t l) {
	write_i(stream, l >> 4 * 8);
	write_i(stream, l & 0xFFFFFFFF);
}
void FFS::write_str(std::ostream& stream, const std::string& s) {
	for(auto c: s) {
		write_c(stream, c);
	}
	write_c(stream, '\0');
}

void FFS::read_c(std::istream& stream, char& c) { 
	if(stream.rdstate() & std::ios_base::eofbit) {
		throw FFS::UnexpectedEOF((uint64_t) stream.tellg());
	}

	stream.get(c);
}

void FFS::read_c(std::istream& stream, uint8_t& c) { 
	// istream.get does not accept uint8_t
	char _c;
	read_c(stream, _c);

	c = _c;
}

void FFS::read_i(std::istream& stream, uint32_t& i) {
	char c1, c2, c3, c4;
	read_c(stream, c1);
	read_c(stream, c2);
	read_c(stream, c3);
	read_c(stream, c4);

	i = ((c1 << (3 * 8)) & 0xFF000000) | ((c2 << (2 * 8)) & 0xFF0000) |
		((c3 << (1 * 8)) & 0xFF00) | ((c4 << (0 * 8)) & 0xFF);
}
void FFS::read_l(std::istream& stream, uint64_t& l) {
	uint32_t i1, i2;
	read_i(stream, i1);
	read_i(stream, i2);

	l = i1;
	l <<= 4 * 8;
	l |= (i2 & 0xFFFFFFFF);
}
void FFS::read_str(std::istream& stream, std::string& s) {
	std::stringstream ss;
	char c;
	read_c(stream, c);
	while(c != '\0') {
		write_c(ss, c);
		read_c(stream, c);
	}
	s = ss.str();
}

size_t FFS::stream_size(std::istream& stream) {
	auto curr_pos = stream.tellg();
	stream.seekg(0, stream.end);
	// must be int so it can go under 0
	size_t length = stream.tellg(); // Tells current location of pointer, i.e. how long the file is
	stream.seekg(curr_pos);
	
	return length;
}

uint64_t FFS::curr_milliseconds() {
	auto now = std::chrono::system_clock::now().time_since_epoch();
	return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}
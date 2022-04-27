#include "functions.h"

#include <cassert>
#include <cstdlib>
#include <iostream>

void FFS::assert_correct_arch() {
	assert(sizeof(char) == 1);
	assert(sizeof(short) == 2);
	assert(sizeof(int) == 4);
	assert(sizeof(float) == 4);
	assert(sizeof(long) == 8);
}

uint32_t FFS::random_int() { return rand(); }

uint32_t FFS::random_int(uint32_t high) { return random_int() % high; }

uint32_t FFS::random_int(uint32_t low, uint32_t high) {
	return low + (random_int((high - low)));
}

long FFS::random_long() {
	return (static_cast<long>(random_int()) << (sizeof(int) * 8)) |
		   random_int();
}

long FFS::random_long(long high) { return random_long() % high; }

long FFS::random_long(long low, long high) {
	return low + (random_long((high - low)));
}

unsigned char FFS::random_byte() { return ((uint32_t)random_int()) % 255; }

void FFS::write_c(std::ostream& stream, char c) { stream.put(c); }
void FFS::write_i(std::ostream& stream, uint32_t i) {
	stream.put((i >> (3 * 8)) & 0xFF);
	stream.put((i >> (2 * 8)) & 0xFF);
	stream.put((i >> (1 * 8)) & 0xFF);
	stream.put((i >> (0 * 8)) & 0xFF);
}
void FFS::write_l(std::ostream& stream, long l) {
	write_i(stream, l >> 4 * 8);
	write_i(stream, l & 0xFFFFFFFF);
}

void FFS::read_c(std::istream& stream, char& c) { stream.get(c); }
void FFS::read_i(std::istream& stream, uint32_t& i) {
	char c1, c2, c3, c4;
	stream.get(c1);
	stream.get(c2);
	stream.get(c3);
	stream.get(c4);

	i = ((c1 << (3 * 8)) & 0xFF000000) | ((c2 << (2 * 8)) & 0xFF0000) |
		((c3 << (1 * 8)) & 0xFF00) | ((c4 << (0 * 8)) & 0xFF);
}
void FFS::read_l(std::istream& stream, long& l) {
	uint32_t i1, i2;
	read_i(stream, i1);
	read_i(stream, i2);

	l = i1;
	l <<= 4 * 8;
	l |= (i2 & 0xFFFFFFFF);
}
#include "functions.h"

#include <cassert>
#include <cstdlib>

void FFS::assert_correct_arch() {
	assert(sizeof(char) == 1);
	assert(sizeof(short) == 2);
	assert(sizeof(int) == 4);
	assert(sizeof(float) == 4);
	assert(sizeof(long) == 8);
}

int FFS::random_int() {
	return rand();
}

int FFS::random_int(int high) {
	return random_int() % high;
}

int FFS::random_int(int low, int high) {
	return low + (random_int((high - low)));
}

long FFS::random_long() {
	return (static_cast<long>(random_int()) << (sizeof(int) * 8)) | random_int();
}

long FFS::random_long(long high) {
	return random_long() % high;
}

long FFS::random_long(long low, long high) {
	return low + (random_long((high - low)));
}

unsigned char FFS::random_byte() {
	return ((unsigned int) random_int()) % 255;
}

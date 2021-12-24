#include "constants.h"

void FFS::assert_correct_arch() {
	assert(sizeof(char) == 1);
	assert(sizeof(short) == 2);
	assert(sizeof(int) == 4);
	assert(sizeof(float) == 4);
	assert(sizeof(long) == 8);
	assert(QuantumRange == 65535);
}
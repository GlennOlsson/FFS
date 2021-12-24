#include <vector>
#include <string>
#include <Magick++.h>

#pragma once

// TODO: Include this
namespace FFS {
	/**
	 * @brief Assert that all types used are of the expected byte length 
	 */
	void assert_correct_arch() {
		assert(sizeof(char) == 1);
		assert(sizeof(short) == 2);
		assert(sizeof(int) == 4);
		assert(sizeof(float) == 4);
		assert(QuantumRange == 65535);
	}

	void encode(std::string path);
	void decode(const std::vector<std::string>& files);
}
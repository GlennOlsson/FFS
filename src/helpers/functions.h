#pragma once

#include <iostream>

namespace FFS {
	/**
	 * @brief Assert that all types used are of the expected byte length 
	 */
	
	void assert_correct_arch();
	 
	uint32_t random_int();
	/** 
	 * Exclusive
	 * 
	*/
	uint32_t random_int(uint32_t high);
	/** 
	 * Low inclusive, high exclusive 
	 * 
	*/
	uint32_t random_int(uint32_t low, uint32_t high);

	long random_long();
	/** 
	 * Exclusive
	 * 
	*/
	long random_long(long high);
	/** 
	 * Low inclusive, high exclusive 
	 * 
	*/
	long random_long(long low, long high);

	unsigned char random_byte();

	void write_c(std::ostream& stream, char c);
	void write_i(std::ostream& stream, uint32_t i);
	void write_l(std::ostream& stream, long l);
	
	void read_c(std::istream& stream, char& c);
	void read_i(std::istream& stream, uint32_t& i);
	void read_l(std::istream& stream, long& l);
}
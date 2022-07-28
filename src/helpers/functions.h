#pragma once

#include <iostream>
#include <string>
namespace FFS {	 
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

	uint64_t random_long();
	/** 
	 * Exclusive
	 * 
	*/
	uint64_t random_long(uint64_t high);
	/** 
	 * Low inclusive, high exclusive 
	 * 
	*/
	uint64_t random_long(uint64_t low, uint64_t high);

	uint8_t random_byte();

	// Generate a string with length characters, only characters 0-9
	std::string random_str(uint32_t length);

	void write_c(std::ostream& stream, uint8_t c);
	void write_i(std::ostream& stream, uint32_t i);
	void write_l(std::ostream& stream, uint64_t l);
	void write_str(std::ostream& stream, std::string s);
	
	void read_c(std::istream& stream, char& c);
	void read_c(std::istream& stream, uint8_t& c);
	void read_i(std::istream& stream, uint32_t& i);
	void read_l(std::istream& stream, uint64_t& l);
	void read_str(std::istream& stream, std::string& s);

	// Get the number of characters in a stream
	size_t stream_size(std::istream&);
}
#pragma once

namespace FFS {
	/**
	 * @brief Assert that all types used are of the expected byte length 
	 */
	
	void assert_correct_arch();
	 
	int random_int();
	/** 
	 * Exclusive
	 * 
	*/
	int random_int(int high);
	/** 
	 * Low inclusive, high exclusive 
	 * 
	*/
	int random_int(int low, int high);

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
}
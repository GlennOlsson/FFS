#include "helpers.h"

#include <iostream>

bool streams_eq(std::basic_istream<char>& a, std::basic_istream<char>& b) {
	std::istreambuf_iterator<char> it1(a);
    std::istreambuf_iterator<char> it2(b);
	
	//Second argument is end-of-range iterator
	return std::equal(it1,std::istreambuf_iterator<char>(),it2); 
}

#include "file_coder.h"

#include "../helpers/constants.h"
#include "../helpers/functions.h"

#include <iostream>
#include <Magick++.h>
#include <string>
#include <math.h>
#include <fstream>
#include <cstdarg>
#include <vector>

/**
 * @brief Assert header is "FFS" followed by length, and return length
 * 
 * @param component_pointer the pointer to each component
 * @return the length of data 
 */
int assert_header(Magick::Quantum*& component_pointer) {

	unsigned short version_nr = *(component_pointer++);

	assert(version_nr == FFS_FILE_VERSION);

	unsigned short component1 = *(component_pointer++);
	unsigned short component2 = *(component_pointer++);
	unsigned short component3 = *(component_pointer++);

	assert(((component1 >> 8) & 0xFF) == 'F');
	assert((component1 & 0xFF) == 'F');
	assert(((component2 >> 8) & 0xFF) == 'S');

	unsigned short b1 = component2 & 0xFF;
	unsigned short b2 = (component3 >> 8) & 0xFF;
	unsigned short b3 = component3 & 0xFF;

	// does not have to be unsigned as only 24 bits, need to use all 
	// 32 to worry about that
	int length = (b1 << 2 * 8) | (b2 << 8) | b3;
	return length;
}

void decode_file(Magick::Image& image, std::ofstream& file_stream) {
	Magick::Pixels pixel_view(image);

	Magick::Geometry image_size = image.size();

	// Pixels is a 3-packed array of rgb values, one Quantum (2 bytes) per component
	Magick::Quantum *component_pointer = pixel_view.get(0, 0, image_size.width(), image_size.height());

	int length = assert_header(component_pointer);

	// Stores 2 bytes of data per pixel in current_value
	int byte_index = 0;

	while(byte_index < length) {
		short bytes = *(component_pointer++);

		char b1 = (bytes >> 8) & 0xFF;
		char b2 = bytes & 0xFF;

		file_stream.put(b1);
		// If should only add one more byte, skip this 
		if(byte_index + 1 < length)
			file_stream.put(b2);

		byte_index += 2;
	}
}

void FFS::decode(const std::vector<std::string>& files){

	std::ofstream file_stream("out.nosync/output", std::ifstream::binary);
	if (!file_stream) {
		std::cerr << "Cannot output to file out.nosync/output" << std::endl;
		return;
	}

	Magick::Image image;
	// std::string filename;
	for(std::string filename: files) {
		// filename = *it;
		image = Magick::Image(filename);
		decode_file(image, file_stream);
	}
}
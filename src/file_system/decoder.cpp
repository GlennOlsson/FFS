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
uint32_t assert_header(Magick::Quantum*& component_pointer) {

	uint16_t version_nr = *(component_pointer++);

	assert(version_nr == FFS_FILE_VERSION);

	uint16_t component1 = *(component_pointer++);
	uint16_t component2 = *(component_pointer++);
	uint16_t component3 = *(component_pointer++);

	assert(((component1 >> 8) & 0xFF) == 'F');
	assert((component1 & 0xFF) == 'F');
	assert(((component2 >> 8) & 0xFF) == 'S');

	uint16_t b1 = component2 & 0xFF;
	uint16_t b2 = (component3 >> 8) & 0xFF;
	uint16_t b3 = component3 & 0xFF;

	// does not have to be unsigned as only 24 bits, need to use all 
	// 32 to worry about that
	uint32_t length = (b1 << 2 * 8) | (b2 << 8) | b3;
	return length;
}

void decode_file(Magick::Image& image, std::ostream& output_stream) {
	Magick::Pixels pixel_view(image);

	Magick::Geometry image_size = image.size();

	// Pixels is a 3-packed array of rgb values, one Quantum (2 bytes) per component
	Magick::Quantum *component_pointer = pixel_view.get(0, 0, image_size.width(), image_size.height());

	uint32_t length = assert_header(component_pointer);

	// Stores 2 bytes of data per pixel in current_value
	uint32_t byte_index = 0;

	while(byte_index < length) {
		uint16_t bytes = *(component_pointer++);

		char b1 = (bytes >> 8) & 0xFF;
		char b2 = bytes & 0xFF;

		FFS::write_c(output_stream, b1);
		// If should only add one more byte, skip this 
		if(byte_index + 1 < length)
			FFS::write_c(output_stream, b2);

		byte_index += 2;
	}
}

void FFS::decode(const std::vector<std::string>& files, std::ostream& file_stream){

	Magick::Image image;
	// std::string filename;
	for(std::string filename: files) {
		// filename = *it;
		image = Magick::Image(filename + "." + FFS_IMAGE_TYPE);
		decode_file(image, file_stream);
	}
}
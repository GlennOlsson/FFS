#include "file_coder.h"

#include "../helpers/constants.h"
#include "../helpers/functions.h"
#include "../exceptions/exceptions.h"

#include <iostream>
#include <Magick++.h>
#include <string>
#include <math.h>
#include <fstream>
#include <cstdarg>
#include <vector>

/**
 * @brief Assert header is "FFS" followed by length, and return length
 * Header structure is described in doc/Binary-Structures.md
 * 
 * @param component_pointer the pointer to each component
 * @return the length of data 
 */
uint32_t assert_header(Magick::Quantum*& component_pointer) {

	uint16_t comp1 = *(component_pointer++);
	uint16_t comp2 = *(component_pointer++);

	bool ok_1 = ((comp1 >> 8) & 0xFF) == 'F';
	bool ok_2 = (comp1 & 0xFF) == 'F';
	bool ok_3 = ((comp2 >> 8) & 0xFF) == 'S';

	if(!ok_1 || !ok_2 || !ok_3)
		throw FFS::BadFFSHeader("Bad letters");
	
	uint8_t ffs_version = comp2 & 0xFF;

	if(ffs_version != FFS_FILE_VERSION)
		throw FFS::BadFFSHeader("Bad version");

	uint16_t comp;
	uint64_t timestamp = 0;
	for(int i = 1; i <= 4; ++i) {
		comp = *(component_pointer++);
		timestamp |= comp << (64 - (16 * i));
	}

	uint32_t length = 0;
	for(int i = 1; i <= 2; ++i) {
		comp = *(component_pointer++);
		length |= comp << (32 - (16 * i));
	}

	return length;
}

void decode_file(Magick::Image& image, std::ostream& output_stream) {
	Magick::Pixels pixel_view(image);

	Magick::Geometry image_size = image.size();

	// Pixels is a 3-packed array of rgb values, one Quantum (2 bytes) per component
	Magick::Quantum* component_pointer = pixel_view.get(0, 0, image_size.width(), image_size.height());

	uint32_t length = assert_header(component_pointer);

	// Stores 2 bytes of data per pixel in current_value
	uint32_t byte_index = 0;

	while(byte_index < length) {
		uint16_t bytes = *(component_pointer++);

		uint8_t b1 = (bytes >> 8) & 0xFF;
		uint8_t b2 = bytes & 0xFF;

		FFS::write_c(output_stream, b1);
		// If should only add one more byte, skip this 
		if(byte_index + 1 < length)
			FFS::write_c(output_stream, b2);

		byte_index += 2;
	}
}

void FFS::decode(const std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> blobs, std::ostream& file_stream){

	Magick::Image image;
	// std::string filename;
	for(auto blob: *blobs) {
		image = Magick::Image(*blob);
		decode_file(image, file_stream);
	}
}
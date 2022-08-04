#include "file_coder.h"

#include "../helpers/constants.h"
#include "../helpers/functions.h"
#include "../exceptions/exceptions.h"

#include "crypto.h"

#include <iostream>
#include <Magick++.h>
#include <string>
#include <math.h>
#include <fstream>
#include <cstdarg>
#include <vector>
#include <memory>

/**
 * @brief Assert header is "FFS" followed by length, and return length
 * Header structure is described in doc/Binary-Structures.md
 * 
 * @param component_pointer the pointer to each component
 * @return the length of data 
 */
uint32_t assert_header(Magick::Quantum*& component_pointer) {

	uint16_t comp1 = component_pointer[0];
	uint16_t comp2 = component_pointer[1];

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
		comp = component_pointer[1 + i];
		timestamp |= comp << (64 - (16 * i));
	} // last component = 1 + 4, next is 6

	std::cout << "timestamp: " << timestamp << std::endl;

	std::cout << "comp[5]: " << (short) component_pointer[5] << std::endl;
	std::cout << "comp[6]: " << (short) component_pointer[6] << std::endl;
	std::cout << "comp[7]: " << (short) component_pointer[7] << std::endl;

	uint32_t length = 0;
	for(int i = 1; i <= 2; ++i) {
		comp = component_pointer[5 + i];
		length |= comp << (32 - (16 * i));
		std::cout << "comp " << i << ": " << ((short) comp) << std::endl;
		std::cout << "length " << i << ": " << (length) << std::endl;
	}

	return length;
}

void decode_file(Magick::Image& image, std::ostream& output_stream) {
	Magick::Pixels pixel_view(image);

	Magick::Geometry image_size = image.size();

	// Pixels is a 3-packed array of rgb values, one Quantum (2 bytes) per component
	Magick::Quantum* encrypted_components = pixel_view.get(0, 0, image_size.width(), image_size.height());

	std::cout << "enc comp[5]: " << (short) encrypted_components[5] << std::endl;
	std::cout << "enc comp[6]: " << (short) encrypted_components[6] << std::endl;
	std::cout << "enc comp[7]: " << (short) encrypted_components[7] << std::endl;

	size_t total_pixels = image_size.width() * image_size.height();

	// 3 Quantum per pixel, 2 bytes per Quantum
	size_t data_count = total_pixels * 3 * sizeof(Magick::Quantum);
	auto component_pointer = (Magick::Quantum*) FFS::Crypto::decrypt((const void*) encrypted_components, data_count);

	std::cout << "bef comp[5]: " << (short) component_pointer[5] << std::endl;
	std::cout << "bef comp[6]: " << (short) component_pointer[6] << std::endl;
	std::cout << "bef comp[7]: " << (short) component_pointer[7] << std::endl;

	uint32_t length = assert_header(component_pointer);

	std::cout << "length: " << length << std::endl;

	// Stores 2 bytes of data per pixel in current_value
	uint32_t pixel_index = 8;

	uint32_t added_bytes = 0;
	while(added_bytes < length) {
		uint16_t bytes = component_pointer[pixel_index];

		uint8_t b1 = (bytes >> 8) & 0xFF;
		uint8_t b2 = bytes & 0xFF;

		std::cout << "b1: " << (char) b1 << ", b2: " << (char) b2 << std::endl;

		FFS::write_c(output_stream, b1);
		// If b1 was last byte, don't add b2. Will break at next condition check
		if(added_bytes + 1 < length)
			FFS::write_c(output_stream, b2);

		pixel_index += 1;
		added_bytes += 2;
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
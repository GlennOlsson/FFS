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
uint32_t assert_header(char* data) {

	bool ok_1 = data[0] == 'F';
	bool ok_2 = data[1] == 'F';
	bool ok_3 = data[2] == 'S';

	if(!ok_1 || !ok_2 || !ok_3)
		throw FFS::BadFFSHeader("Bad letters");

	uint8_t ffs_version = data[3];

	if(ffs_version != FFS_FILE_VERSION)
		throw FFS::BadFFSHeader("Bad version");

	// TODO: Do something with me
	// uint64_t timestamp = *(uint64_t*) &data[4];

	uint32_t length = *(uint32_t*) &data[12];

	return length;
}

void decode_file(Magick::Image& image, std::ostream& output_stream) {
	Magick::Pixels pixel_view(image);

	Magick::Geometry image_size = image.size();

	// Pixels is a 3-packed array of rgb values, one Quantum (2 bytes) per component
	auto encrypted_pixel_data = pixel_view.get(0, 0, image_size.width(), image_size.height());
	uint32_t encrypted_data_len = ((unsigned short) encrypted_pixel_data[0] << 16) | (unsigned short) encrypted_pixel_data[1];

	if(encrypted_data_len > FFS_MAX_FILE_SIZE)
		throw FFS::BadFFSFile("Encrypted data length to big");
	

	// Skip 4 first bytes == stores encrypted data length
	unsigned char* encrypted_data = new unsigned char[encrypted_data_len];

	auto pixel_index = 2;
	auto byte_index = 0;
	while(byte_index < encrypted_data_len) {
		auto s = (unsigned short) encrypted_pixel_data[pixel_index++];
		encrypted_data[byte_index] = (s >> 8) & 0xFFFF;
		if(byte_index + 1 < encrypted_data_len)
			encrypted_data[byte_index + 1] = s & 0xFFFF;

		byte_index += 2;
	}

	auto decryption = FFS::Crypto::decrypt(encrypted_data, encrypted_data_len);
	auto raw_data = (char*) decryption.ptr;

	uint32_t length = assert_header(raw_data);
	// Write length amount of bytes to stream
	uint32_t data_index = FFS_HEADER_SIZE;
	while(data_index < length + FFS_HEADER_SIZE)
		FFS::write_c(output_stream, raw_data[data_index++]);
	
	delete[] encrypted_data;
	// Cannot delete void*
	delete[] (char*) decryption.ptr;
}

void FFS::decode(const FFS::blobs_t blobs, std::ostream& file_stream){

	Magick::Image image;
	// std::string filename;
	for(auto blob: *blobs) {
		image = Magick::Image(*blob);
		decode_file(image, file_stream);
	}
}
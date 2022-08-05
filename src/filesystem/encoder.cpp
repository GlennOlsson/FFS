#include "file_coder.h"

#include "../helpers/constants.h"
#include "../helpers/functions.h"

#include "crypto.h"

#include <iostream>
#include <Magick++.h>
#include <string>
#include <math.h>
#include <fstream>
#include <cassert>
#include <vector>
#include <memory>

// Header structure is described in doc/Binary-Structures.md
void save_header(char* data, uint32_t length) {

	data[0] = 'F';
	data[1] = 'F';
	data[2] = 'S';

	data[3] = FFS_FILE_VERSION;

	const auto timestamp = std::chrono::system_clock::now();
	uint64_t nanos_since_epoch = std::chrono::duration_cast<std::chrono::nanoseconds>(
		timestamp.time_since_epoch()
	).count();

	// Interpret data[4] as a uint64_t pointer, and save timestamp
	*((uint64_t*) &data[4]) = nanos_since_epoch;

	// Interpret data[12] as a uint32_t pointer, and save length
	*((uint32_t*) &data[12]) = length;
}

void encode_data(std::istream& input_stream, uint32_t stream_len, char* data) {
	save_header(data, stream_len);
	
	uint32_t byte_index = FFS_HEADER_SIZE;
	while(input_stream)
		FFS::read_c(input_stream, data[byte_index++]);
}

std::shared_ptr<Magick::Blob> FFS::create_image(std::istream& input_stream, uint32_t length) {
	assert(QuantumRange == 65535);

	// Bytes required for header and file
	uint32_t data_bytes = length + FFS_HEADER_SIZE;

	assert(data_bytes <= FFS_MAX_FILE_SIZE);

	char* data = new char[data_bytes];
	// Write header and all stream data to data ptr
	encode_data(input_stream, length, data);

	auto encryption = FFS::Crypto::encrypt(data, data_bytes);

	auto encrypted_data = (char*) encryption.ptr;
	auto encrypted_len = encryption.len;

	// Minumum required bytes for image. Length of encrypted data (4 bytes) + Encrypted data
	uint32_t required_bytes = 4 + encrypted_len;

	// Assume 16bit depth for each component
	// 2 bytes per component, 3 components per pixel, 2*3
	uint32_t required_pixels = ceil((double) required_bytes / 6.0);

	// Find closest square that can fit the pixels
	uint32_t width = ceil(sqrt(required_pixels));
	uint32_t height = ceil((double) required_pixels / (double) width);
	uint32_t total_pixels = width * height;

	// Total bytes in image, is data bytes + filler bytes
	// Per pixel, 3 colors (quantum). Per Quantum, 2 bytes. i.e. 6 bytes per pixel
	uint32_t total_bytes = total_pixels * 6;

	Magick::Image image(Magick::Geometry(width, height), Magick::Color("white"));

	image.magick(FFS_IMAGE_TYPE);
	image.quality(100);
	
	image.type(Magick::TrueColorType);
	image.modifyImage(); // Used to make sure there's only one reference to image pixels

	Magick::Pixels pixel_view(image);
	// Pixels is a 3-packed array of rgb values, one Quantum (2 bytes) per component
	Magick::Quantum* component_pointer = pixel_view.get(0, 0, width, height);

	component_pointer[0] = (encrypted_len >> 16) & 0xFFFF;
	component_pointer[1] = encrypted_len & 0xFFFF;
	// component_pointer[0] = (uint32_t) 64;

	// First 4 bytes == 2 components save the encrypted data lenght
	uint32_t byte_index = 4;

	// Keeps two bytes, most significan bytes at most significant position
	uint16_t current_value;

	uint8_t b;
	while(byte_index < total_bytes) {
		if(byte_index < encrypted_len + 4) {
			b = encrypted_data[byte_index - 4];
		}
		else {
			b = FFS::Crypto::random_c();
		}

		if(byte_index % 2 == 0) {
			current_value = (b << 8) & 0xFF00;
		} else { // mod == 1
			current_value |= (b & 0xFF);
			component_pointer[byte_index / 2] = current_value;
		}
		byte_index += 1;
	}

	pixel_view.sync();

	std::shared_ptr<Magick::Blob> blob = std::make_shared<Magick::Blob>();
	image.write(blob.get());

	delete[] data;
	// Cannot delete void*
	delete[] (char*) encryption.ptr;

	return blob;
}

std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> FFS::encode(std::istream& file_stream) {
	int length = FFS::stream_size(file_stream);

	uint32_t out_file_index = 0;

	auto blobs = std::make_shared<std::vector<std::shared_ptr<Magick::Blob>>>();

	while(length > 0) {
		uint32_t out_file_size = std::min(FFS_MAX_FILE_SIZE - FFS_HEADER_SIZE, (int) length);

		std::shared_ptr<Magick::Blob> blob = create_image(file_stream, out_file_size);
		blobs->push_back(blob);

		length -= out_file_size;
		out_file_index++;
	}

	return blobs;
}

std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> FFS::encode(std::string input_path) {
	std::ifstream file_stream(input_path, std::ifstream::binary);
	if (!file_stream) {
		std::cerr << "no file " << input_path << std::endl;

		auto empty_list = std::make_shared<std::vector<std::shared_ptr<Magick::Blob>>>();
		return empty_list;
	}

	return encode(file_stream);
}
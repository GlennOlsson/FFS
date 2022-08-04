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
void save_header(Magick::Quantum*& component_pointer, uint32_t length) {

	component_pointer[0] = ('F' << 8) | 'F';
	component_pointer[1] = ('S' << 8) | FFS_FILE_VERSION;

	const auto timestamp = std::chrono::system_clock::now();

	uint64_t nanos_since_epoch = std::chrono::duration_cast<std::chrono::nanoseconds>(
		timestamp.time_since_epoch()
	).count();

	component_pointer[2] = (nanos_since_epoch >> 48) & 0xFFFF;
	component_pointer[3] = (nanos_since_epoch >> 32) & 0xFFFF;
	component_pointer[4] = (nanos_since_epoch >> 16) & 0xFFFF;
	component_pointer[5] = nanos_since_epoch & 0xFFFF;

	component_pointer[6] = (length >> 16) & 0xFFFF;
	component_pointer[7] = length & 0xFFFF;
}

std::shared_ptr<Magick::Blob> FFS::create_image(std::istream& input_stream, uint32_t length) {
	assert(QuantumRange == 65535);

	// Bytes required for header and file
	uint32_t min_bytes = length + FFS_HEADER_SIZE;

	assert(min_bytes <= FFS_MAX_FILE_SIZE);

	// Assume 16bit depth for each component
	// 2 bytes per component, 3 components per pixel, 2*3
	uint32_t required_pixels = ceil((double) min_bytes / 6.0);

	// Find closest square that can fit the pixels
	uint32_t width = ceil(sqrt(required_pixels));
	uint32_t height = ceil((double) required_pixels / (double) width);
	uint32_t total_pixels = width * height;

	// Bytes required to change in output image
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

	//Save header and length of file 
	save_header(component_pointer, length);

	// First pixel saves header
	uint32_t component_index = 8;
	uint32_t written_bytes = 8 * 2;

	// Keeps two bytes, most significan bytes at most significant position
	uint16_t current_value;

	uint8_t b;
	while(written_bytes < total_bytes) {
		if(written_bytes < length + FFS_HEADER_SIZE) {
			FFS::read_c(input_stream, b);
		}
		else {
			b = random_byte();
		}

		// If first byte in component, shift to left by one byte
		if(written_bytes % 2 == 0) {
			current_value = (b << 8) & 0xFF00;
		} else { // mod == 1
			current_value |= (b & 0xFF);
			component_pointer[component_index++] = current_value;
		}
		written_bytes += 1;
	}


	size_t data_count = total_bytes;
	auto encrypted_pixels = (Magick::Quantum*) FFS::Crypto::encrypt((const void*) component_pointer, data_count);
	
	memcpy(component_pointer, encrypted_pixels, total_bytes);

	pixel_view.sync();

	std::shared_ptr<Magick::Blob> blob = std::make_shared<Magick::Blob>();
	image.write(blob.get());
	
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
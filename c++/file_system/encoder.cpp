#include "file_coder.h"

#include "../helpers/constants.h"
#include "../helpers/functions.h"

#include <iostream>
#include <Magick++.h>
#include <string>
#include <math.h>
#include <fstream>
#include <cassert>

// Bytes required for header
#define HEADER_SIZE 8

/*
	Sets first pixel as header

	Header consists of first pixel with 4 * 2 = 8 bytes, in order:
	VERSION_NR (2 bytes)
	"F"	"F"
	"S"	L1
	L2	L3

	Where L1-L3 is the length divided up into 3 bytes with L1 being most significant
	and L3 least significant. "F" and "S" is the char value of these strings.

	Assumes that length is not greater than 2^24 (16 million)

*/
void save_header(Magick::Quantum*& component_pointer, int length) {
	char F = 'F';
	char S = 'S';

	// Use 3 bytes to represent length of content
	// Move bits to look at to far-right, and 0 all other digits
	unsigned char L1 = (length >> 16) & 0xFF;
	unsigned char L2 = (length >> 8) & 0xFF;
	unsigned char L3 = length & 0xFF;

	unsigned short version_nr = FFS_FILE_VERSION;
	unsigned short component1 = (F << 8) | F;
	unsigned short component2 = (S << 8) | L1;
	unsigned short component3 = (L2 << 8) | L3;

	// Increment pointer after assignment
	*(component_pointer++) = version_nr;
	*(component_pointer++) = component1;
	*(component_pointer++) = component2;
	*(component_pointer++) = component3;
}

void FFS::create_image(std::string output_name, std::istream& file_stream, int length) {
	assert(QuantumRange == 65535);

	// Bytes required for header and file
	int min_bytes = length + HEADER_SIZE;

	// Assume 16bit depth for each component
	// 2 bytes per component, 3 components per pixel, 2*3
	int required_pixels = ceil((double) min_bytes / 6.0);

	// Find closest square that can fit the pixels
	int width = ceil(sqrt(required_pixels));
	int height = ceil((double) required_pixels / (double) width);
	int total_pixels = width * height;

	// Bytes required to change in output image
	// file length + header size + filler pixels
	int total_bytes = total_pixels * 6;

	Magick::Image image(Magick::Geometry(width, height), Magick::Color("white"));

	image.magick(FFS_IMAGE_TYPE);
	image.quality(100);
	
	image.type(Magick::TrueColorType);
	image.modifyImage(); // Used to make sure there's only one reference to image pixels

	Magick::Pixels pixel_view(image);
	// Pixels is a 3-packed array of rgb values, one Quantum (2 bytes) per component
	Magick::Quantum *component_pointer = pixel_view.get(0, 0, width, height);
	
	//Save header and length of file 
	save_header(component_pointer, length);

	// First pixel saves header
	int byte_index = HEADER_SIZE;

	// Keeps two bytes, most significan bytes at most significant position
	unsigned short current_value;

	char b;
	while(byte_index < total_bytes) {
		if(byte_index < (length + HEADER_SIZE)) {
			file_stream.get(b);
		}
		else {
			b = random_byte();
		}

		// If first byte in component, shift to left by one byte
		if(byte_index % 2 == 0) {
			current_value = (b << 8) & 0xFF00;
		} else { // mod == 1
			current_value |= (b & 0xFF);
			*(component_pointer++) = current_value;
		}
 
		byte_index += 1;
	}
	pixel_view.sync();

	image.write(output_name + "." + FFS_IMAGE_TYPE);
}

void FFS::encode(std::string path) {
	std::ifstream file_stream(path, std::ifstream::binary);
	if (!file_stream) {
		std::cerr << "no file " << path << std::endl;
		return;
	}

	// length of file:
	file_stream.seekg (0, file_stream.end);
	int length = file_stream.tellg(); // Tells current location of pointer, i.e. how long the file is
	file_stream.seekg (0, file_stream.beg);

	int out_file_index = 0;
	while(length > 0) {
		int out_file_size = std::min(FFS_MAX_FILE_SIZE, length);
		std::string out_file_name = "out.nosync/img" + std::to_string(out_file_index);
		//TODO: Make concurrent?
		create_image(out_file_name, file_stream, out_file_size);
		length -= out_file_size;
		out_file_index++;
	}
}

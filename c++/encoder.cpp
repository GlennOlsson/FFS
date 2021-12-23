#include <iostream>
#include <Magick++.h>
#include <string>
#include <math.h>
#include <fstream>
#include <cassert>
#include <cstdlib>

using namespace Magick;
using namespace std;

// Bytes required for header
#define HEADER_SIZE 6

// Maximum possible possible number stored in 24 bits == header of output image
#define MAX_FILE_SIZE 16777216

unsigned char random_char() {
	return rand() % 255;
}

/*
	Sets first pixel as header

	Header consists of first pixel with 3 * 2 = 6 bytes, in order:
	"F"	"F"
	"S"	L1
	L2	L3

	Where L1-L3 is the length divided up into 3 bytes with L1 being most significant
	and L3 least significant. "F" and "S" is the char value of these strings.

	Assumes that length is not greater than 2^24 (16 million)

*/
void save_header(Quantum*& component_pointer, int length) {
	char F = 'F';
	char S = 'S';

	// Use 3 bytes to represent length of content
	// Move bits to look at to far-rigth, and 0 all other digits
	unsigned char L1 = (length >> 16) & 0xFF;
	unsigned char L2 = (length >> 8) & 0xFF;
	unsigned char L3 = length & 0xFF;

	unsigned short component1 = (F << 8) | F;
	unsigned short component2 = (S << 8) | L1;
	unsigned short component3 = (L2 << 8) | L3;

	// Increment pointer after assignment
	*(component_pointer++) = component1;
	*(component_pointer++) = component2;
	*(component_pointer++) = component3;
}

void encode(string path) {
	ifstream file_stream(path, ifstream::binary);
	if (!file_stream) {
		cerr << "no file " << path << endl;
		return;
	}

	// length of file:
	file_stream.seekg (0, file_stream.end);
	int length = file_stream.tellg(); // Tells current location of pointer, i.e. how long the file is
	file_stream.seekg (0, file_stream.beg);

	if(length > MAX_FILE_SIZE) {
		cerr << "File to big" << endl;
		return;
	}

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

	Image image(Geometry(width, height), Color("white"));

	image.magick("png");
	image.quality(100);
	
	image.type(TrueColorType);
	image.modifyImage(); // Used to make sure there's only one reference to image pixels

	Pixels pixel_view(image);
	// Pixels is a 3-packed array of rgb values, one Quantum (2 bytes) per component
	Quantum *component_pointer = pixel_view.get(0, 0, width, height);
	
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
			b = random_char();
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

	image.write("out/img.png");
}

void assert_correct_arch() {
	assert(sizeof(char) == 1);
	assert(sizeof(short) == 2);
	assert(sizeof(int) == 4);
	assert(sizeof(float) == 4);
	assert(QuantumRange == 65535);
}

int main(int argc, char const *argv[]){
	assert_correct_arch();
	InitializeMagick(*argv);

	string filename;
	if(argc > 1) {
		filename = argv[1];
		encode(filename);
	} else {
		encode("out/input");
	}

	return 0;
}
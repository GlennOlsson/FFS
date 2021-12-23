
#include <iostream>
#include <Magick++.h>
#include <string>
#include <math.h>
#include <fstream>
#include <cassert>

using namespace Magick;
using namespace std;

/**
 * @brief Assert header is "FFS" followed by length, and return length
 * 
 * @param component_pointer the pointer to each component
 * @return the length of data 
 */
int assert_header(Quantum*& component_pointer) {
	unsigned short red = *(component_pointer++);
	unsigned short green = *(component_pointer++);
	unsigned short blue = *(component_pointer++);

	assert(((red >> 8) & 0xFF) == 'F');
	assert((red & 0xFF) == 'F');
	assert(((green >> 8) & 0xFF) == 'S');

	unsigned short b1 = green & 0xFF;
	unsigned short b2 = (blue >> 8) & 0xFF;
	unsigned short b3 = blue & 0xFF;

	// does not have to be unsigned as only 24 bits, need to use all 
	// 32 to worry about that
	int length = (b1 << 2 * 8) | (b2 << 8) | b3;
	return length;
}

void decode(string filename){

	Image image(filename);

	Pixels pixel_view(image);

	Geometry image_size = image.size();

	// Pixels is a 3-packed array of rgb values, one Quantum (2 bytes) per component
	Quantum *component_pointer = pixel_view.get(0, 0, image_size.width(), image_size.height());

	int length = assert_header(component_pointer);

	ofstream file_stream("out/output", ifstream::binary);
	if (!file_stream) {
		cerr << "Cannot output to file out/output" << endl;
		return;
	}

	// Stores 2 bytes of data per pixel in current_value
	int byte_index = 0;

	while(byte_index < length) {
		short bytes = *(component_pointer++);

		char b1 = (bytes >> 8) & 0xFF;
		char b2 = bytes & 0xFF;

		file_stream << b1;
		// If should only add one more byte, skip this 
		if(byte_index + 1 < length)
			file_stream << b2;

		byte_index += 2;
	}
}

void assert_correct_arch() {
	assert(sizeof(char) == 1);
	assert(sizeof(short) == 2);
	assert(sizeof(int) == 4);
	assert(QuantumRange == 65535);
}

int main(int argc, char const *argv[]){
	assert_correct_arch();
	InitializeMagick(*argv);

	string filename;
	if(argc > 1) {
		filename = argv[1];
		decode(filename);
	} else {
		decode("out/img.png");
	}

	return 0;
}

#include <iostream>
#include <Magick++.h>
#include <string>
#include <math.h>
#include <fstream>
#include <cassert>

using namespace Magick;
using namespace std;

short read_pixel(int& pixel_index, char& component, Image& img) {
	size_t height = img.rows();
	size_t width = img.columns();

	size_t x = pixel_index % width;
	size_t y = floor(pixel_index / width); // How many times can width fit in index?

	Color current_color = img.pixelColor(x, y);
	short component_value;
	switch (component) {
	case 0:
		component_value = current_color.quantumRed();
		break;

	case 1:
		component_value = current_color.quantumGreen();
		break;
		
	case 2:
		component_value = current_color.quantumBlue();
		break;
	
	default:
		cerr << "COMPONENT TO HIGH: " << component << endl;
		break;
	}

	component++;
	component %= 3;
	if(component == 0)
		pixel_index++;
	
	return component_value;
}

/**
 * @brief Assert header is "FFS" followed by length, and return length
 * 
 * @param img the image
 * @return the lenght of data 
 */
int assert_header(Image& img) {
	auto pixel = img.pixelColor(0, 0);

	unsigned short red = pixel.quantumRed();
	unsigned short green = pixel.quantumGreen();
	unsigned short blue = pixel.quantumBlue();

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

	int length = assert_header(image);

	ofstream file_stream("out/output", ifstream::binary);
	if (!file_stream) {
		cerr << "Cannot output to file out/output" << endl;
		return;
	}

	// Stores 2 bytes of data per pixel in current_value
	int byte_index = 0;

	//Value 0-2, r, g or b
	char current_component = 0;

	// First pixel saves header
	int current_pixel = 1;

	while(byte_index < length) {
		short bytes = read_pixel(current_pixel, current_component, image);

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
#include <iostream>
#include <Magick++.h>
#include <string>
#include <math.h>
#include <fstream>
#include <cassert>

using namespace Magick;
using namespace std;

/**
 * @brief Convert 0-255 values of rgba to Magick::Color
 * 
 * @param r red
 * @param g green
 * @param b blue
 * @param a alpha
 * @return the color 
 */
Color rgba(int r, int g, int b, int a) {
	auto conv = [](int i) {
		double frac = (double) i / 255.0;
		int val = round(frac * QuantumRange);
		return val;
	};

	Quantum red = conv(r);
	Quantum green = conv(g);
	Quantum blue = conv(b);
	Quantum alpha = conv(a);

	return Color(red, green, blue, alpha);
}

Color rgb(int r, int g, int b) {
	return rgba(r, g, b, 255);
}

void set_pixel(int& pixel_index, char& component, Image& img, short& value) {
	size_t height = img.rows();
	size_t width = img.columns();

	size_t x = pixel_index % width;
	size_t y = floor(pixel_index / width); // How many times can width fit in index?

	cout << "X: " << x << ", y: " << y << endl;

	Color current_color = img.pixelColor(x, y);
	switch (component) {
	case 0:
		current_color.quantumRed(value);
		break;

	case 1:
		current_color.quantumGreen(value);
		break;
		
	case 2:
		current_color.quantumBlue(value);
		break;
	
	default:
		cerr << "COMPONENT TO HIGH: " << component << endl;
		break;
	}
	img.pixelColor(x, y, current_color);

	component++;
	component %= 3;
	if(component == 0)
		pixel_index++;
}

/*
	Header consists of first pixel with 3 * 2 = 6 bytes, in order:
	"F"	"F"
	"S"	L1
	L2	L3

	Where L1-L3 is the length divided up into 3 bytes with L1 being most significant
	and L3 least significant. "F" and "S" is the char value of these strings.

	Assumes that length is not greater than 2^24 (16 million)

*/
void save_header(Image& img, int length) {
	char F = 'F';
	char S = 'S';

	// Move bits to look at to far-rigth, and 0 all other digits
	char L1 = (length >> 16) & 0xFF;
	char L2 = (length >> 8) & 0xFF;
	char L3 = length & 0xFF;

	short comp1 = (F << 8) | F;
	short comp2 = (S << 8) | L1;
	short comp3 = (L2 << 8) | L3;

	int index = 0;
	char component = 0;

	set_pixel(index, component, img, comp1);
	set_pixel(index, component, img, comp2);
	set_pixel(index, component, img, comp3);
}

void encode(string path) {
	cout << "read file " << path << endl;

	ifstream file_stream(path, ifstream::binary);
	if (!file_stream) {
		cerr << "no file " << path << endl;
		return;
	}

	// length of file:
	file_stream.seekg (0, file_stream.end);
	int length = file_stream.tellg(); // Tells current location of pointer, i.e. how long the file is
	file_stream.seekg (0, file_stream.beg);

	// Assume 16bit depth for each pixel
	// lenght is in bytes, can store 2 bytes per pixel
	int pixels_for_file = ceil((double) length / 2.0);

	// Find closest square that can fit the pixels
	int dimension = ceil(sqrt(pixels_for_file));

	cout << "dim: " << dimension << ", " << dimension << endl;

	Image image(Geometry(dimension, dimension), Color("white"));

	cout << "created img " << endl;

	//Save header and length of file 
	save_header(image, length);

	// Stores 2 bytes of data per pixel in current_value
	int byte_index = 0;

	//Value 0-2, r, g or b
	char current_component = 0;

	// First pixel saves header
	int current_pixel = 1;

	short current_value;

	char b;
	while(byte_index < length) {
		file_stream.get(b);

		// If first byte in component, shift to left by one byte
		if(byte_index % 2 == 0) {
			current_value = b << 8;
		} else { // mod == 1
			current_value |= b;
			set_pixel(current_pixel, current_component, image, current_value);
		}
 
		cout << "b for i=" << byte_index << " ; " << b << endl;
		byte_index += 1;
	}
	
	image.magick("png");

	image.write("file_name_explicit_extension.png");

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
		encode(filename);
	} else {
		encode("out/input");
	}

	return 0;
}
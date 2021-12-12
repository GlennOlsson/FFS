
#include <iostream>
#include <string>
#include <Magick++.h>

using namespace Magick;
using namespace std;

int main(int argc, char const *argv[]){
	InitializeMagick(*argv);

	Image my_image = *new Image();
	my_image.read("file_name_explicit_extension.png");

	Color pixel_sample;
	pixel_sample = my_image.pixelColor(5,5); 
	
	cout << "dec pix: " << string(pixel_sample) << endl;

	return 0;
}
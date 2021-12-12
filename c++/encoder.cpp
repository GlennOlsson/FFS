#include <iostream>
#include <Magick++.h>
#include <string>

using namespace Magick;
using namespace std;

int main(int argc, char const *argv[]){
	// InitializeMagick(*argv);


	Image my_image("640x480", "white");

	Quantum r = 200;
	Quantum g = 100;
	Quantum b = 10;

	Color pixel_sample(r,g, b);
	my_image.pixelColor(5,5, pixel_sample); 

	cout << "enc pix: " << string(pixel_sample) << endl;
	
	my_image.magick("png");

	my_image.write("file_name_explicit_extension.png");

	return 0;
}
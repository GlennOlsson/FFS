#include "filesystem/file_coder.h"
#include "filesystem/inode_table.h"
#include "filesystem/fuse.h"

#include "helpers/constants.h"
#include "helpers/functions.h"

#include "user_io/cmd.h"

#include <iostream>
#include <fstream>
#include <string>
#include <Magick++.h>
#include <sstream>

using std::cout;
using std::cerr;
using std::endl;


int encode_main(int argc, char *argv[]){
	Magick::InitializeMagick(*argv);

	std::string filename;
	if(argc > 1) {
		filename = argv[1];
	} else {
		filename = "out.nosync/input";
	}
	std::vector<Magick::Blob*>* blobs = FFS::encode(filename);

	int i = 0;
	for(Magick::Blob* b: *blobs) {
		Magick::Image img(*b);

		std::stringstream out_name;
		out_name << "out.nosync/img" << i++ << "." << FFS_IMAGE_TYPE;

		img.write(out_name.str());
	}

	return 0;
}

int decode_main(int argc, char *argv[]){
	Magick::InitializeMagick(*argv);

	std::string filename;
	std::vector<Magick::Blob*> input_list;
	if(argc > 1) {
		for(int i = 1; i < argc; i++) {
			Magick::Image img = Magick::Image(argv[i]);
			Magick::Blob* blob = new Magick::Blob();
			img.write(blob);
			input_list.push_back(blob);
		}
	} else {
		std::stringstream out_name;
		out_name << "out.nosync/img0." << FFS_IMAGE_TYPE;

		Magick::Image img = Magick::Image(out_name.str());	
		Magick::Blob* blob = new Magick::Blob();
		img.write(blob);
		input_list.push_back(blob);
	}

	std::ofstream file_stream("out.nosync/output", std::ofstream::binary);
	if (!file_stream) {
		std::cerr << "Cannot output to file out.nosync/output" << std::endl;
		return 1;
	}
	FFS::decode(&input_list, file_stream);

	return 0;
}

void fs_interact(int argc, char *argv[]){
	interact();
}

int main(int argc, char *argv[]) {

	if(argc < 2) {
		cerr << "Need to supply argument" << endl;
		return 1;
	}

	std::string argument = argv[1];

	if(argument == "encode") {
		encode_main(argc - 1, ++argv);
	} else if(argument == "decode") {
		decode_main(argc - 1, ++argv);
	} else if(argument == "fs") {
		fs_interact(argc - 1, ++argv);
	} else if(argument == "fuse") {
		return FFS::FUSE::start(argc, argv);
	} else {
		cerr << "Argument not covered: " << argv[1] << endl;
		return 1;
	}

	return 0;
}
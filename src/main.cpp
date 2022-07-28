#include "filesystem/file_coder.h"
#include "filesystem/inode_table.h"
#include "filesystem/fuse.h"

#include "helpers/constants.h"
#include "helpers/functions.h"

#include "user_io/cmd.h"

// #include "api/curl.h"

#include <iostream>
#include <fstream>
#include <string>
#include <Magick++.h>
#include <sstream>
#include <memory>

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
	auto blobs = FFS::encode(filename);

	int i = 0;
	for(auto b: *blobs) {
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
	auto input_list = std::make_shared<std::vector<std::shared_ptr<Magick::Blob>>>();
	if(argc > 1) {
		for(int i = 1; i < argc; i++) {
			Magick::Image img = Magick::Image(argv[i]);
			auto blob = std::make_shared<Magick::Blob>();
			img.write(blob.get());
			input_list->push_back(blob);
		}
	} else {
		std::stringstream out_name;
		out_name << "out.nosync/img0." << FFS_IMAGE_TYPE;

		Magick::Image img = Magick::Image(out_name.str());	
		auto blob = std::make_shared<Magick::Blob>();
		img.write(blob.get());
		input_list->push_back(blob);
	}

	std::ofstream file_stream("out.nosync/output", std::ofstream::binary);
	if (!file_stream) {
		std::cerr << "Cannot output to file out.nosync/output" << std::endl;
		return 1;
	}

	FFS::decode(input_list, file_stream);

	return 0;
}

void fs_interact(int argc, char *argv[]){
	interact();
}

void tweet() {
	cout << "nah" << endl;
}

int main(int argc, char *argv[]) {

	if(argc < 2) {
		cerr << "Need to supply argument" << endl;
		return 1;
	}

	// FFS::API::Curl::init();

	std::string argument = argv[1];

	if(argument == "encode")
		encode_main(argc - 1, ++argv);
	else if(argument == "decode")
		decode_main(argc - 1, ++argv);
	else if(argument == "fs")
		fs_interact(argc - 1, ++argv);
	else if(argument == "fuse")
		return FFS::FUSE::start(argc - 1, argv + 1);
	else if(argument == "tweet")
		tweet();
	else {
		cerr << "Argument not covered: " << argv[1] << endl;
		return 1;
	}

	return 0;
}
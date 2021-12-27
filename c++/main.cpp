#include "file_system/file_coder.h"
#include "file_system/inode_table.h"

#include "helpers/constants.h"
#include "helpers/functions.h"

#include <iostream>
#include <string>

using std::cout;
using std::cerr;
using std::endl;


int encode_main(int argc, char const *argv[]){
	FFS::assert_correct_arch();
	Magick::InitializeMagick(*argv);

	std::string filename;
	if(argc > 1) {
		filename = argv[1];
		FFS::encode(filename);
	} else {
		FFS::encode("out.nosync/input");
	}

	return 0;
}

int decode_main(int argc, char const *argv[]){
	FFS::assert_correct_arch();
	Magick::InitializeMagick(*argv);

	std::string filename;
	std::vector<std::string> input_list;
	if(argc > 1) {
		for(int i = 1; i < argc; i++) {
			input_list.push_back(argv[i]);
		}
	} else {
		input_list.push_back("out.nosync/img0.png");
	}
	FFS::decode(input_list);

	return 0;
}

void inode_table_main() {
	
}

int main(int argc, char const *argv[]) {

	if(argc < 2) {
		cerr << "Need to supply argument" << endl;
		return 1;
	}

	std::string argument = argv[1];

	if(argument == "encode") {
		encode_main(argc - 1, ++argv);
	} else if(argument == "decode") {
		decode_main(argc - 1, ++argv);
	} else if(argument == "inode_table") {
		cout << "Inode table" << endl;
	} else {
		cerr << "Argument not covered: " << argv[1] << endl;
		return 1;
	}

	return 0;
}
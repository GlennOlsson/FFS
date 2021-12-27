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



	// #include "catch.hpp"

#include "file_system/inode_table.h"

#include "helpers/functions.h"

#include <unordered_map>
#include <vector>

#include <iostream>

using std::cout;
using std::endl;

// TEST_CASE("Can construct inode table with equal maps", "[inode_table]") {

int main(int argc, char const *argv[]){
	std::vector<unsigned long> v1;
	std::vector<unsigned long> v2;

	cout << "v1 == v2" << endl;
	cout << (v1 == v2) << endl;
	
	v1.push_back(15l);

	cout << "v1 == v2, 2" << endl;
	cout << (v1 == v2) << endl;


	std::unordered_map<unsigned int, FFS::InodeEntry&> m;

	
	for(int i = 0; i < 10; i++) {

		cout << "Entry " << i << endl;

		unsigned int rand_inode_id = FFS::random_int();

		//Random length between 10 and 10000 bytes
		int rand_length = FFS::random_int(10, 10000);

		std::vector<unsigned long> rand_blocks;

		int rand_block_counts = FFS::random_int(1, 100);
		cout << "random block count: " << rand_block_counts << endl; 
		for(int j = 0; j < rand_block_counts; j++) {
			unsigned long rand_tweet_id = FFS::random_long();
			
			rand_blocks.push_back(rand_tweet_id);
		}

		FFS::InodeEntry* entry = new FFS::InodeEntry(rand_length, rand_blocks);
	
		m.insert_or_assign(rand_inode_id, *entry);
	}

	// FFS::InodeEntry& entry3 = m.at(rand_inode_id);
	// // cout << "Creating table with " << &m << ", size: " << entry3.tweet_blocks.size() << " --- last inode: " << rand_inode_id << endl;

	FFS::InodeTable table(m);

	// cout << "last inode: " << rand_inode_id << endl;
	// FFS::InodeEntry& entry2 = m.at(rand_inode_id);
	// cout << "entry size: " << entry2.tweet_blocks.size() << endl;

	// cout << "ASSERT" << endl;

	cout << "M count" << m.size() << ", t size " << table.entries.size() << endl;;

	for(auto m_p: m) {
		cout << "m1: " << m_p.first << ", m2 length: " << m_p.second.size() << endl;
		cout << "t[m1].count: " << table.entries.count(m_p.first) << endl; 
		FFS::InodeEntry e = m_p.second;
		cout << e.length << endl;
		cout << "eq: " << (table.entries.at(m_p.first) == m_p.second) << endl; //CHECK SIZE; POINTERS HERE
	}

	bool maps_eq = m == table.entries;

	cout << "Maps equal? " << (maps_eq ? "t" : "f") << endl;

	// REQUIRE(maps_eq);

	cout << "REQUIRE RAN " << endl;
}

// 	if(argc < 2) {
// 		cerr << "Need to supply argument" << endl;
// 		return 1;
// 	}

// 	std::string argument = argv[1];

// 	if(argument == "encode") {
// 		encode_main(argc - 1, ++argv);
// 	} else if(argument == "decode") {
// 		decode_main(argc - 1, ++argv);
// 	} else if(argument == "inode_table") {
// 		cout << "Inode table" << endl;
// 	} else {
// 		cerr << "Argument not covered: " << argv[1] << endl;
// 		return 1;
// 	}

// 	return 0;
// }
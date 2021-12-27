#include "catch.hpp"

#include "../file_system/inode_table.h"

#include "../helpers/functions.h"

#include <unordered_map>
#include <vector>

#include <iostream>

using std::cout;
using std::endl;

TEST_CASE("Can construct inode table with equal maps", "[inode_table]") {
	std::unordered_map<unsigned int, FFS::InodeEntry&> m;

	for(int i = 0; i < 10; i++) {

		cout << "Entry " << i << endl;

		unsigned int rand_inode_id = FFS::random_int();

		//Random length between 10 and 10000 bytes
		int rand_length = FFS::random_int(10, 10000);

		std::vector<unsigned long> rand_blocks = std::vector<unsigned long>();

		int rand_block_counts = FFS::random_int(1, 100);
		for(int j = 0; j < rand_block_counts; j++) {
			unsigned long rand_tweet_id = FFS::random_long();
			
			rand_blocks.push_back(rand_tweet_id);
		}

		FFS::InodeEntry* entry = new FFS::InodeEntry(rand_length, rand_blocks);

		m.insert_or_assign(rand_inode_id, *entry);
	}

	cout << "Creating table" << endl;

	FFS::InodeTable table(m);

	cout << "ASSERT" << endl;

	cout << "M count" << m.size() << ", t size " << table.entries.size() << endl;;

	for(auto m_p: m) {
		cout << "m1: " << m_p.first << ", m2 length: " << m_p.second.size() << endl;
		cout << "t[m1].counties: " << table.entries.count(m_p.first) << endl; 
		cout << "eqss: " << (table.entries.at(m_p.first) == m_p.second) << endl; 
	}

	cout << "out of loop" << endl;

	bool maps_eq = m == table.entries;

	cout << "Maps equal? " << (maps_eq ? "t" : "f") << endl;

	REQUIRE(maps_eq);

	cout << "REQUIRE RAN " << endl;
}
#include "catch.hpp"

#include "../file_system/inode_table.h"

#include "../helpers/functions.h"

#include <unordered_map>
#include <vector>
#include <string>

#include <iostream>

FFS::InodeTable* create_table(std::unordered_map<unsigned int, FFS::InodeEntry*>* m = nullptr) {
	if(m == nullptr)
		m = new std::unordered_map<unsigned int, FFS::InodeEntry*>();

	for(int i = 0; i < 10; i++) {
		unsigned int rand_inode_id = FFS::random_int();

		//Random length between 10 and 10000 bytes
		int rand_length = FFS::random_int(10, 10000);

		std::vector<unsigned long>* rand_blocks = new std::vector<unsigned long>();

		int rand_block_counts = FFS::random_int(1, 100);
		for(int j = 0; j < rand_block_counts; j++) {
			unsigned long rand_tweet_id = FFS::random_long();
			
			rand_blocks->push_back(rand_tweet_id);
		}

		FFS::InodeEntry* entry = new FFS::InodeEntry(rand_length, rand_blocks);

		m->insert_or_assign(rand_inode_id, entry);
	}

	FFS::InodeTable* table = new FFS::InodeTable(m);
	return table;
}

TEST_CASE("Can construct inode table with equal maps", "[inode_table]") {
	std::unordered_map<unsigned int, FFS::InodeEntry*>* m = new std::unordered_map<unsigned int, FFS::InodeEntry*>();

	FFS::InodeTable* table = create_table(m);

	bool maps_eq = *m == *table->entries;

	REQUIRE(maps_eq);
}

TEST_CASE("Sterlizing and desterlizing inode table creates same table", "[inode_table]") {
	FFS::InodeTable* table = create_table();

	std::string inode_table_output = "out.nosync/inode_table";

	table->save(inode_table_output);
	FFS::InodeTable* desterilized_table = FFS::InodeTable::load(inode_table_output);

	bool tables_eq = *table == *desterilized_table;

	REQUIRE(tables_eq);
}
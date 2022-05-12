#include "catch.hpp"

// So we can access private fields of the class
#define private public

#include "../src/filesystem/inode_table.h"

#include "../src/helpers/functions.h"

#include <map>
#include <vector>
#include <string>

#include <iostream>

FFS::InodeTable* create_table(std::map<uint32_t, FFS::InodeEntry*>* m = nullptr) {
	if(m == nullptr)
		m = new std::map<uint32_t, FFS::InodeEntry*>();

	// 10 files
	for(uint32_t i = 0; i < 10; i++) {
		uint32_t rand_inode_id = FFS::random_int();

		//Random length between 10 and 10000 bytes
		uint32_t rand_length = FFS::random_int(10, 10000);

		std::vector<uint64_t>* rand_blocks = new std::vector<uint64_t>();

		uint32_t rand_block_counts = FFS::random_int(1, 100);
		for(uint32_t j = 0; j < rand_block_counts; j++) {
			uint64_t rand_post_id = FFS::random_long();
			
			rand_blocks->push_back(rand_post_id);
		}

		FFS::InodeEntry* entry = new FFS::InodeEntry(rand_length, rand_blocks);

		m->insert_or_assign(rand_inode_id, entry);
	}

	FFS::InodeTable* table = new FFS::InodeTable(m);
	return table;
}

TEST_CASE("Can construct inode table with equal maps", "[inode_table]") {
	std::map<uint32_t, FFS::InodeEntry*>* m = new std::map<uint32_t, FFS::InodeEntry*>();

	FFS::InodeTable* table = create_table(m);

	bool maps_eq = *m == *table->entries;

	REQUIRE(maps_eq);
}

TEST_CASE("Sterlizing and desterlizing inode table creates same table", "[inode_table]") {
	FFS::InodeTable* table = create_table();

	std::string inode_table_output = "out.nosync/inode_table.png";

	Magick::Blob* b = table->blob();

	FFS::InodeTable* desterilized_table = FFS::InodeTable::from_blob(b);

	bool tables_eq = *table == *desterilized_table;

	REQUIRE(tables_eq);
}
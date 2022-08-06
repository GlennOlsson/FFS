#include "catch.hpp"

// So we can access private fields of the class
#define private public

#include "../src/filesystem/inode_table.h"
#include "../src/filesystem/storage.h"

#include "../src/helpers/functions.h"

#include <map>
#include <vector>
#include <string>

#include <iostream>

std::shared_ptr<FFS::InodeTable> create_table(std::shared_ptr<std::map<uint32_t, std::shared_ptr<FFS::InodeEntry>>> m = nullptr) {
	if(m == nullptr)
		m = std::make_shared<std::map<uint32_t, std::shared_ptr<FFS::InodeEntry>>>();

	// 10 files
	for(uint32_t i = 0; i < 10; i++) {
		FFS::inode_t rand_inode = FFS::random_int();

		//Random length between 10 and 10000 bytes
		uint32_t rand_length = FFS::random_int(10, 10000);

		auto rand_blocks = std::make_shared<std::vector<FFS::post_id_t>>();

		uint32_t rand_block_counts = FFS::random_int(1, 100);
		for(uint32_t j = 0; j < rand_block_counts; j++) {
			auto rand_post_id_t = FFS::random_str(10);
			
			rand_blocks->push_back(rand_post_id_t);
		}

		uint8_t is_dir = FFS::random_byte() % 2; // will be either 0 or 1

		auto entry = std::make_shared<FFS::InodeEntry>(rand_length, rand_blocks, is_dir);

		m->insert_or_assign(rand_inode, entry);
	}

	auto table = std::make_shared<FFS::InodeTable>(m);
	return table;
}

TEST_CASE("Can construct inode table with equal maps", "[inode_table]") {
	auto m = std::make_shared<std::map<uint32_t, std::shared_ptr<FFS::InodeEntry>>>();

	auto table = create_table(m);

	bool maps_eq = *m == *table->entries;

	REQUIRE(maps_eq);
}

TEST_CASE("Sterlizing and desterlizing inode table creates same table", "[inode_table]") {
	auto table = create_table();

	std::string inode_table_output = "out.nosync/inode_table.png";

	auto blobs = FFS::Storage::blobs(*table);

	auto deserialized_table = FFS::Storage::itable_from_blob(blobs->front());

	bool tables_eq = *table == *deserialized_table;

	REQUIRE(tables_eq);
}
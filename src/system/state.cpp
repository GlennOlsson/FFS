#include "state.h"

#include "../filesystem/storage.h"

#include <iostream>

FFS::InodeTable* FFS::State::inode_table = nullptr;
FFS::post_id FFS::State::inode_table_id = 0;

FFS::InodeTable* FFS::State::get_inode_table() {
	if(FFS::State::inode_table == nullptr) {
		InodeTable* table = new InodeTable();
		State::inode_table = table;
		std::cout << "created table, saving" << std::endl;
		save_table();
		std::cout << "Saved table" << std::endl;
	}

	return State::inode_table;
}


void FFS::State::save_table() {
	FFS::InodeTable* table = get_inode_table();
	if(table == nullptr) {
		std::cout << "Cannot save table, nullptr" << std::endl;
		return;
	}

	auto blob = FFS::Storage::blob(*table);

	FFS::Storage::save_file(inode_table_id, blob);
}
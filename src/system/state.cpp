#include "state.h"

#include "../filesystem/storage.h"

#include <iostream>

FFS::InodeTable* FFS::State::inode_table = nullptr;
FFS::post_id FFS::State::inode_table_id = 0;

FFS::InodeTable* FFS::State::get_inode_table() {
	if(FFS::State::inode_table == nullptr) {
		InodeTable* table = new InodeTable();
		State::inode_table = table;

		save_table();
	}

	return State::inode_table;
}


void FFS::State::save_table() {
	FFS::InodeTable* table = get_inode_table();
	if(table == nullptr) {
		std::cout << "Cannot save table, nullptr" << std::endl;
		return;
	}

	auto blob = table->blob();

	FFS::Storage::save_file(inode_table_id, blob);
}
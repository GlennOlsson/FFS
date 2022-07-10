#include "state.h"

#include "../filesystem/storage.h"

#include <iostream>

FFS::InodeTable* FFS::State::inode_table = nullptr;
FFS::post_id FFS::State::inode_table_id = 0;

FFS::InodeTable* FFS::State::get_inode_table() {
	if(FFS::State::inode_table == nullptr) {
		// try to load from storage, else create new
		try {
			std::vector<post_id> posts = {inode_table_id};
			auto blobs = FFS::Storage::get_file(&posts);
			State::inode_table = FFS::Storage::itable_from_blobs(blobs);
		} catch(std::exception& e) {
			State::inode_table = new InodeTable();
			save_table();
		}
	}

	return State::inode_table;
}


void FFS::State::save_table() {
	FFS::InodeTable* table = get_inode_table();
	if(table == nullptr) {
		std::cout << "Cannot save table, nullptr" << std::endl;
		return;
	}

	auto blobs = FFS::Storage::blobs(*table);

	// DANGEROUS: Assuming only one blob for inode table, _should_ be fine!
	FFS::Storage::save_file(inode_table_id, blobs->front());
}
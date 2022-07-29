#include "state.h"

#include "../filesystem/storage.h"

#include "../exceptions/exceptions.h"

#include <iostream>

std::shared_ptr<FFS::InodeTable> FFS::State::inode_table = nullptr;
FFS::post_id FFS::State::inode_table_id = "";

std::shared_ptr<FFS::InodeTable> FFS::State::get_inode_table() {
	if(FFS::State::inode_table == nullptr) {
		// try to load from storage, else create new
		std::shared_ptr<Magick::Blob> blob;
		FFS::post_id new_id = "";
		try {
			auto post_id = FFS::Storage::get_inode_table();	
			blob = FFS::Storage::get_file(post_id);
		} catch (FFS::Exception& e) {
			blob = nullptr;
		}

		// If could get blob, save new post_id and inode table
		if(blob != nullptr) {
			FFS::State::inode_table_id = new_id;
			State::inode_table = FFS::Storage::itable_from_blob(blob);
		} else {
			State::inode_table = std::make_shared<InodeTable>();
			save_table();
		}
	}

	return State::inode_table;
}

void FFS::State::clear_inode_table() {
	FFS::State::inode_table.reset();
	FFS::State::inode_table = nullptr;
}

void FFS::State::save_table() {
	auto old_id = FFS::State::inode_table_id;

	std::shared_ptr<FFS::InodeTable> table = get_inode_table();
	if(table == nullptr) {
		std::cerr << "Cannot save table, nullptr" << std::endl;
		return;
	}

	auto blobs = FFS::Storage::blobs(*table);

	// DANGEROUS: Assuming only one blob for inode table, _should_ be fine!
	FFS::Storage::upload_file(blobs->front(), true);

	// If old id existed, remove old post
	if(old_id.size() > 2)
		FFS::Storage::remove_post(old_id);
}
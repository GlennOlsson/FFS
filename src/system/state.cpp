#include "state.h"

#include "../filesystem/storage.h"

#include "../exceptions/exceptions.h"

#include "../helpers/logger.h"

#include <iostream>
#include <set>

std::shared_ptr<FFS::InodeTable> FFS::State::inode_table = nullptr;
FFS::post_id_t FFS::State::inode_table_id = "";

std::shared_ptr<FFS::InodeTable> FFS::State::get_inode_table() {
	if(FFS::State::inode_table == nullptr) {
		// try to load from storage, else create new
		std::shared_ptr<Magick::Blob> blob;
		FFS::post_id_t new_id = "";
		try {
			new_id = FFS::Storage::get_inode_table();	
			blob = FFS::Storage::get_file(new_id);
		} catch (FFS::Exception& e) {
			blob = nullptr;
		}

		// If could get blob, save new post_id_t and inode table
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
	// Delete from service
	if(FFS::State::inode_table_id.size() > 2)
		FFS::Storage::remove_post(FFS::State::inode_table_id);
	
	FFS::State::inode_table_id = "";
}

void FFS::State::save_table() {
	FFS::log << "Saving table" << std::endl;
	auto old_id = FFS::State::inode_table_id;

	std::shared_ptr<FFS::InodeTable> table = get_inode_table();
	if(table == nullptr) {
		FFS::err << "Cannot save table, nullptr" << std::endl;
		return;
	}

	auto blobs = FFS::Storage::blobs(*table);
	
	// DANGEROUS: Assuming only one blob for inode table, _should_ be fine!
	FFS::State::inode_table_id = FFS::Storage::upload_file(blobs->front(), true);

	// If old id existed, remove old post
	if(old_id.size() > 2)
		FFS::Storage::remove_post(old_id, true);
}

std::set<FFS::post_id_t> deleting_posts;

void FFS::State::deleting(FFS::post_id_t post_id) {
	deleting_posts.insert(post_id);
}

bool FFS::State::is_deleting(FFS::post_id_t post_id) {
	return deleting_posts.contains(post_id);
}

void FFS::State::deleted(FFS::post_id_t post_id) {
	deleting_posts.erase(post_id);
}

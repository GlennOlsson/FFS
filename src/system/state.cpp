	#include "state.h"

#include "../filesystem/storage.h"

#include "../exceptions/exceptions.h"

#include "../helpers/logger.h"

#include <iostream>
#include <set>

std::shared_ptr<FFS::InodeTable> FFS::State::inode_table = nullptr;
FFS::post_id_t FFS::State::inode_table_id = "";

void create_new_table() {
	FFS::State::inode_table = std::make_shared<FFS::InodeTable>();
	FFS::State::save_table();
}

std::shared_ptr<FFS::InodeTable> FFS::State::get_inode_table() {
	if(FFS::State::inode_table == nullptr) {
		// try to load from storage, else create new
		FFS::blob_t blob;
		FFS::post_id_t current_id = "";
		try {
			auto pair = FFS::Storage::get_inode_table();
			blob = pair.first;
			current_id = pair.second;
		} catch (const FFS::Exception& e) {
			FFS::log << "Error getting inode table: " << e.what() << std::endl;
			blob = nullptr;
		} catch (const std::exception& e2) {
			FFS::log << "Non-FFS error getting inode table: " << e2.what() << std::endl;
			blob = nullptr;
		}

		// If could get blob, save new post_id_t and inode table
		if(blob != nullptr) {
			try {
				FFS::State::inode_table = FFS::Storage::itable_from_blob(blob);
			}   catch (const std::exception& e2) {
				// For instance, if decryption does not work
				FFS::log << "Non-FFS error decoding inode table: " << e2.what() << std::endl;
				create_new_table();
			}
			FFS::State::inode_table_id = current_id;
		} else {
			create_new_table();
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

	FFS::log << "Old table id: '" << old_id << "'" << std::endl;

	std::shared_ptr<FFS::InodeTable> table = FFS::State::inode_table;
	if(table == nullptr) {
		FFS::err << "Cannot save table, nullptr" << std::endl;
		return;
	}

	FFS::log << "New table address: '" << table << "'" << std::endl;

	auto blobs = FFS::Storage::blobs(*table);

	FFS::log << "Decoded new table, blobs address: '" << blobs << "'" << std::endl;
	FFS::log << "Blobs count: '" << blobs->size() << "'" << std::endl;
	
	// DANGEROUS: Assuming only one blob for inode table, _should_ be fine!
	FFS::State::inode_table_id = FFS::Storage::upload_file(blobs->front(), true);

	FFS::log << "New table id: '" << inode_table_id << "'" << std::endl;

	// If old id existed, remove old post
	if(old_id.size() > 2) {
		FFS::Storage::remove_post(old_id, true);
		FFS::log << "Removing old table" << std::endl;
	} else {
		FFS::log << "Not removing old table, id = " << old_id << std::endl;
	}
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

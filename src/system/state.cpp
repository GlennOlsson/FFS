#include "state.h"

FFS::InodeTable* FFS::State::inode_table = nullptr;

FFS::InodeTable* FFS::State::get_inode_table() {
	if(FFS::State::inode_table == nullptr) {
		InodeTable* table = new InodeTable();
		State::inode_table = table;
	}

	return State::inode_table;
}
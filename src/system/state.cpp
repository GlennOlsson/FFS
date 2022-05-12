#include "state.h"


FFS::InodeTable* FFS::FFSState::get_inode_table() {
	if(FFS::FFSState::inode_table == nullptr) {
		InodeTable* table = new InodeTable();
		FFSState::inode_table = table;
	}

	return FFSState::inode_table;
}
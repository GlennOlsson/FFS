#include "../filesystem/inode_table.h"

#pragma once

namespace FFS {
class FFSState {
private:
	static InodeTable* inode_table;

public:
	// Check if has a inode table, else fetch from social media
	static InodeTable* get_inode_table();
};

InodeTable* FFSState::inode_table = nullptr;
}
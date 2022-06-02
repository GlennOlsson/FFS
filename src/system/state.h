#include "../filesystem/inode_table.h"

#ifndef STATE_H
#define STATE_H

namespace FFS {
class State {
private:
	static InodeTable* inode_table;

public:
	// Check if has a inode table, else fetch from social media
	static InodeTable* get_inode_table();
};
}

#endif
#include "../filesystem/inode_table.h"

#ifndef STATE_H
#define STATE_H

namespace FFS {
class State {
private:
	static std::shared_ptr<InodeTable> inode_table;
	static post_id inode_table_id;

public:
	// Check if has a inode table, else fetch from social media
	static std::shared_ptr<InodeTable> get_inode_table();
	// Remove current inode table from memory, requires re-fetch next time
	static void clear_inode_table();
	static void save_table();
};
}

#endif
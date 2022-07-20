#include "cache.h"

#include "../helpers/types.h"

#include <memory>
#include <Magick++.h>
#include <ostream>
#include <map>
#include <list>
#include <iterator>

#define inode_queue_t std::list<FFS::inode_id>

std::map<FFS::inode_id, std::shared_ptr<FFS::Directory>> dir_cache;
// Keeps track of the order the directory were added in, i.e. which should be removed if the cache is full
inode_queue_t dir_cache_queue;

std::map<FFS::inode_id, std::shared_ptr<std::ostream>> file_cache;
// Keeps track of the order the files were added in, i.e. which should be removed if the cache is full
inode_queue_t file_cache_queue;

void remove_if_in(inode_queue_t& queue, FFS::inode_id inode) {
	auto it = queue.begin();
	while(it != queue.end()) {
		if(*it == inode) {
			queue.erase(it);
			return;
		}
		++it;
	}
}

// --- DIR --- 

void FFS::cache(FFS::inode_id inode, std::shared_ptr<FFS::Directory> dir) {
	auto insert_result = dir_cache.insert_or_assign(inode, dir);
	// If false, means that it was assigned, i.e. inode already existed before. Could be in cache
	if(!insert_result.second) {
		remove_if_in(dir_cache_queue, inode);
	}

	// Add inode to beginning of queue
	dir_cache_queue.push_front(inode);

	// If queue is now full (should max be SIZE + 1), remove last element
	if(dir_cache_queue.size() > DIRECTORY_CACHE_SIZE)
		dir_cache_queue.pop_back();
}

std::shared_ptr<FFS::Directory> FFS::get_dir(FFS::inode_id inode) {
	// would preferably put this inode in the beginning of the queue as it was used,
	// but with list it will be pretty slow. TODO: can we use another container or
	// can we use the slow list-operation? Probably faster than downloading from twitter anyway
	return dir_cache.contains(inode) ? dir_cache.at(inode) : nullptr;
}

// --- FILE ----

void FFS::cache(FFS::inode_id inode, std::shared_ptr<std::ostream> file) {
	auto insert_result = file_cache.insert_or_assign(inode, file);
	// If false, means that it was assigned, i.e. inode already existed before. Could be in cache
	if(!insert_result.second) {
		remove_if_in(file_cache_queue, inode);
	}

	// Add inode to beginning of queue
	file_cache_queue.push_front(inode);

	// If queue is now full (should max be SIZE + 1), remove last element
	if(file_cache_queue.size() > FILE_CACHE_SIZE)
		file_cache_queue.pop_back();
}

std::shared_ptr<std::ostream> FFS::get_file(FFS::inode_id inode) {
	// see comment under get_dir
	return file_cache.contains(inode) ? file_cache.at(inode) : nullptr;
}


void FFS::invalidate(FFS::inode_id inode) {
	// Doesn't matter if the inode is a dir or a file, nothing throws
	remove_if_in(dir_cache_queue, inode);
	remove_if_in(file_cache_queue, inode);

	dir_cache.erase(inode);
	file_cache.erase(inode);
}
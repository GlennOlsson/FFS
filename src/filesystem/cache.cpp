#include "cache.h"

#include "../helpers/types.h"
#include "../helpers/constants.h"

#include <memory>
#include <Magick++.h>
#include <istream>
#include <map>
#include <list>
#include <iterator>
#include <iostream>

std::shared_ptr<FFS::Directory> root_dir = nullptr;

std::map<FFS::post_id, std::shared_ptr<Magick::Blob>> post_cache;
// Keeps track of the order the post were added in, i.e. which should be removed if the cache is full
std::list<FFS::post_id> post_cache_queue;


template<class T>
void remove_if_in(std::list<T>& queue, T elem) {
	auto it = queue.begin();
	while(it != queue.end()) {
		if(*it == elem) {
			queue.erase(it);
			return;
		}
		++it;
	}
}

void FFS::Cache::cache_root(std::shared_ptr<FFS::Directory> root) {
	root_dir = root;
}

std::shared_ptr<FFS::Directory> FFS::Cache::get_root() {
	return root_dir;
}

void FFS::Cache::invalidate_root() {
	root_dir = nullptr;
}

void FFS::Cache::cache(FFS::post_id post_id, std::shared_ptr<Magick::Blob> blob) {
	auto insert_result = post_cache.insert_or_assign(post_id, blob);
	// If false, means that it was assigned, i.e. inode already existed before. Could be in cache
	if(!insert_result.second) {
		remove_if_in(post_cache_queue, post_id);
	}

	// Add inode to beginning of queue
	post_cache_queue.push_front(post_id);

	// If queue is now full (should max be SIZE + 1), remove last element
	if(post_cache_queue.size() > 50) {
		auto rm_inode = post_cache_queue.back();
		post_cache.erase(rm_inode);
		post_cache_queue.pop_back();
	}
}

std::shared_ptr<Magick::Blob> FFS::Cache::get(FFS::post_id post_id) {
	return post_cache.contains(post_id) ? post_cache.at(post_id) : nullptr;
}

void FFS::Cache::invalidate(FFS::post_id post_id) {
	remove_if_in(post_cache_queue, post_id);
	post_cache.erase(post_id);
}

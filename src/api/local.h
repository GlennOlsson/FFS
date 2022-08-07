#include "../config.h"

#ifdef DEBUG
#ifndef FFS_LOCAL_API_H
#define FFS_LOCAL_API_H

#include "../helpers/types.h"

#include <memory>
#include <istream>
#include <string>
// Local filesystem, used for debugging instead of using actual OWS
namespace FFS::API::Local {
	FFS::post_id_t save_file(const std::string& from_path, bool is_inode_table);
	std::shared_ptr<std::istream> get_file(const FFS::post_id_t&);
	FFS::post_id_t get_inode_post_id();
	void delete_file(const FFS::post_id_t&);
}

#endif // FFS_LOCAL_API_H
#endif // DEBUG
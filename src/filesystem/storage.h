#pragma once

#include "../helpers/types.h"

#include <string>
#include <vector>
#include <Magick++.h>

// API to save, get and delete FFS files on online services and where the data is stored
namespace FFS::Storage {
	std::vector<post_id> upload_file(std::vector<Magick::Blob*>& blobs);
	void get_file(post_id id);
}
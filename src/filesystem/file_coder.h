#pragma once

#include "../helpers/types.h"

#include <vector>
#include <string>
#include <Magick++.h>
#include <memory>

#define FFS_HEADER_SIZE 16

namespace FFS {

	FFS::blobs_t encode(std::istream& file_stream);
	FFS::blobs_t encode(std::string input_path);
	FFS::blob_t create_image(std::istream& input_stream, uint32_t length);
	
	void decode(const FFS::blobs_t blobs, std::ostream& file_stream);
}
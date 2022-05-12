#pragma once

#include <vector>
#include <string>
#include <Magick++.h>

namespace FFS {

	std::vector<Magick::Blob*>* encode(std::string input_path);
	Magick::Blob* create_image(std::istream& input_stream, uint32_t length);
	
	void decode(const std::vector<Magick::Blob*>* blobs, std::ostream& file_stream);
}
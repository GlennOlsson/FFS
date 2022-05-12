#pragma once

#include <vector>
#include <string>
#include <Magick++.h>

namespace FFS {

	// TODO: Return list of Blob instead
	std::vector<Magick::Blob> encode(std::string input_path);
	// TODO: Return list of Blob instead
	std::vector<Magick::Blob> create_image(std::istream& file_stream, uint32_t length);
	
	// TODO: Take list of Blob instead
	void decode(const std::vector<Magick::Blob>& blobs, std::ostream& file_stream);
}
#pragma once

#include <vector>
#include <string>
#include <Magick++.h>
#include <memory>

namespace FFS {

	std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> encode(std::istream& file_stream);
	std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> encode(std::string input_path);
	std::shared_ptr<Magick::Blob> create_image(std::istream& input_stream, uint32_t length);
	
	void decode(const std::shared_ptr<std::vector<std::shared_ptr<Magick::Blob>>> blobs, std::ostream& file_stream);
}
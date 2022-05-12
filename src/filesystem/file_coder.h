#pragma once

#include <vector>
#include <string>
#include <Magick++.h>

namespace FFS {

	std::vector<Magick::Blob*>* encode(std::string input_path);
	Magick::Blob* create_image(std::istream& input_stream, uint32_t length);
	
	void decode(const std::vector<Magick::Blob*>* blobs, std::ostream& file_stream);

	// TODO: DEPRECATED, remove and fix methods using them
	// Replaced by create_image
	void _save_encoded_image(std::string out_path, std::istream& file_stream, uint32_t length);
	
	// Replaced by decode
	void _read_encoded_image(const std::string& path, std::ostream& file_stream);
}
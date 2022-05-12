#pragma once

#include <vector>
#include <string>
#include <Magick++.h>

namespace FFS {

	// TODO: Return list of Blob instead
	std::vector<Magick::Blob*>* encode(std::string input_path);
	// TODO: Return list of Blob instead
	Magick::Blob* create_image(std::istream& input_stream, uint32_t length);
	
	// TODO: Take list of Blob instead
	void decode(const std::vector<Magick::Blob*>& blobs, std::ostream& file_stream);


	void _save_encoded_image(std::string out_path, std::istream& file_stream, uint32_t length);
	void _read_encoded_image(const std::string& path, std::ostream& file_stream);
}
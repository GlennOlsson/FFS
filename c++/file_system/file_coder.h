#include <vector>
#include <string>
#include <Magick++.h>

#pragma once

// TODO: Include this
namespace FFS {

	void encode(std::string path);
	void create_image(std::string output_name, std::istream& file_stream, int length);

	void decode(const std::vector<std::string>& files);
}
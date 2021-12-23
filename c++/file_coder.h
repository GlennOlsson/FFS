#include <string>

#pragma once

// TODO: Include this
namespace FFS {
	void assert_arch();
	void encode(std::string path);
	void decode(std::string paths...);
}
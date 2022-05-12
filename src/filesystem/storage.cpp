#include "storage.h"

#include "../helpers/types.h"
#include "../helpers/functions.h"
#include "../helpers/constants.h"

#include <Magick++.h>
#include <vector>
#include <string>
#include <sstream>

std::string path_of(FFS::post_id id) {
	std::stringstream path_stream;
	path_stream << FFS_TMP_FS_PATH << "/ffs_" << std::to_string(id) << "." << FFS_IMAGE_TYPE;
	return path_stream.str();
}

std::vector<FFS::post_id>* upload_file(std::vector<Magick::Blob*>* blobs) {
	std::vector<FFS::post_id>* posts = new std::vector<FFS::post_id>(blobs->size());

	for(Magick::Blob* blob: *blobs) {
		// Assume no collision as it's 64-bit, i.e. 1.8e19 choices
		FFS::post_id id = FFS::random_long();
		std::string path = path_of(id);

		posts->push_back(id);

		Magick::Image img(*blob);
		img.write(path);
	}

	return posts;
}

Magick::Blob* get_file(FFS::post_id id) {
	std::string path = path_of(id);
	Magick::Image img(path);

	Magick::Blob* blob = new Magick::Blob();
	img.write(blob);

	return blob;
}
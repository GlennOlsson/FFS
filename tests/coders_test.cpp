#include "catch.hpp"

#include "../src/filesystem/file_coder.h"

#include "../src/helpers/constants.h"
#include "../src/helpers/functions.h"
#include "../src/exceptions/exceptions.h"

#include <fstream>
#include <iostream>
#include <string>
#include <Magick++.h>

#define INPUT_FILE_PATH std::string("/tmp/ffs_in")
#define OUTPUT_FILE_PATH std::string("/tmp/ffs_out")
#define ENCODED_IMAGE_PATH std::string("/tmp/ffs_img")

bool files_eq(const std::string& f1, const std::string& f2) {
	std::ifstream s1(f1, std::ifstream::ate | std::ifstream::binary);
	std::ifstream s2(f2, std::ifstream::ate | std::ifstream::binary);

	std::istreambuf_iterator<char> it1(s1);
    std::istreambuf_iterator<char> it2(s2);
	
	//Second argument is end-of-range iterator
	return std::equal(it1,std::istreambuf_iterator<char>(),it2); 
}

void save_to_file(std::string& content, const std::string& file) {
	std::ofstream stream(file);
	stream << content;
}

std::vector<std::string>& image_paths(uint32_t count) {
	std::vector<std::string>* v = new std::vector<std::string>;
	for(uint32_t i = 0; i < count; i++) {
		v->push_back(ENCODED_IMAGE_PATH + std::to_string(i));
	}
	return *v;
}

TEST_CASE("Encode and decode plain text", "[coders]") {
	
	std::string s = "Hello my name is glenn";
	save_to_file(s, INPUT_FILE_PATH);

	auto out_blobs = FFS::encode(INPUT_FILE_PATH);
	
	std::ofstream out_stream(OUTPUT_FILE_PATH);

	FFS::decode(out_blobs, out_stream);
	out_stream.close();

	REQUIRE(files_eq(INPUT_FILE_PATH, OUTPUT_FILE_PATH));
}

TEST_CASE("Encode and decode special character text", "[coders]") {
	
	std::string s = "Bagare Bengtson bakom berget bakar bara brända bullar 🥵🍪⏲🧑‍🍳";
	save_to_file(s, INPUT_FILE_PATH);

	auto out_blobs = FFS::encode(INPUT_FILE_PATH);
	
	std::ofstream out_stream(OUTPUT_FILE_PATH);

	FFS::decode(out_blobs, out_stream);
	out_stream.close();

	REQUIRE(files_eq(INPUT_FILE_PATH, OUTPUT_FILE_PATH));
}

TEST_CASE("Encode and decode a pdf file", "[coders]") {
	
	std::string pdf_path = "tests/assets.nosync/pdf.pdf";

	auto out_blobs = FFS::encode(pdf_path);
	
	std::ofstream out_stream(OUTPUT_FILE_PATH);

	FFS::decode(out_blobs, out_stream);
	out_stream.close();

	REQUIRE(files_eq(pdf_path, OUTPUT_FILE_PATH));
}

TEST_CASE("Encode and decode a image file", "[coders]") {
	
	std::string image_path = "tests/assets.nosync/image.png";

	auto out_blobs = FFS::encode(image_path);
	
	std::ofstream out_stream(OUTPUT_FILE_PATH);

	FFS::decode(out_blobs, out_stream);
	out_stream.close();

	REQUIRE(files_eq(image_path, OUTPUT_FILE_PATH));
}

TEST_CASE("Encode and decode a big movie that requires multiple splitted files", "[coders]") {
	
	std::string movie_path = "tests/assets.nosync/movie.mp4";

	auto out_blobs = FFS::encode(movie_path);
	
	std::ofstream out_stream(OUTPUT_FILE_PATH);

	// Know that the current version of the movie and the codebase should require 6 image files
	FFS::decode(out_blobs, out_stream);
	out_stream.close();

	REQUIRE(files_eq(movie_path, OUTPUT_FILE_PATH));
}

TEST_CASE("Ensure decoder throws when image is not FFS image", "[coders]") {
	std::string image_path = "tests/assets.nosync/image.png";

	std::ofstream out_stream(OUTPUT_FILE_PATH);

	auto blobs = std::make_shared<std::vector<std::shared_ptr<Magick::Blob>>>();

	auto blob = std::make_shared<Magick::Blob>();
	Magick::Image img(image_path);
	img.write(blob.get());

	blobs->push_back(blob);

	try {
		FFS::decode(blobs, out_stream);
		FAIL("Did not throw exception");
	} catch(const FFS::BadFFSFile& b) {
		SUCCEED("Threw exception as expected");
	}
}
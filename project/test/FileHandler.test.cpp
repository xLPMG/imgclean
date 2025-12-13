#include "catch.hpp"

#include "imgclean/FileHandler.hpp"
#include <string>

#ifdef CIMG_FOUND
# define cimg_display 0 // we dont need to display images -> reduce dependencies
# include <CImg.h>
#endif

//! Data taken from 3x3-test.ppm
static const std::vector<uint16_t> expected_pixels = {255, 0, 0, 0, 255, 0,   0,   0,   255, 255, 255, 255, 103, 103,
                                                      103, 0, 0, 0, 0,   255, 255, 255, 0,   255, 255, 255, 0};

TEST_CASE("FileHandler ASCII PPM Loading", "[FileHandler][PPM]")
{
	imgclean::FilePath path = imgclean::FileHandler::make_file_path("../res/3x3-test.ppm");
	imgclean::PPMImage img;
	bool success = imgclean::FileHandler::load_image(path, img);
	REQUIRE(success);
	REQUIRE(img.width == 3);
	REQUIRE(img.height == 3);
	REQUIRE(img.maxval == 255);
	REQUIRE(img.pixels.size() == 3 * 3 * 3); // width * height * 3 (RGB)
	REQUIRE(img.pixels == expected_pixels);
}

TEST_CASE("FileHandler ASCII PPM Saving", "[FileHandler][PPM]")
{
	imgclean::FilePath load_path = imgclean::FileHandler::make_file_path("../res/3x3-test.ppm");
	imgclean::FilePath save_path = imgclean::FileHandler::make_file_path(
		"../build/test_output/3x3-test-output.ppm");
	imgclean::PPMImage img;
	bool load_success = imgclean::FileHandler::load_image(load_path, img);
	REQUIRE(load_success);
	bool save_success = imgclean::FileHandler::save_image(save_path, img);
	REQUIRE(save_success);
}

TEST_CASE("FileHandler PNG Loading", "[FileHandler][PNG]")
{
	imgclean::FilePath path = imgclean::FileHandler::make_file_path("../res/3x3-test.png");
	imgclean::PPMImage img;
	bool success = imgclean::FileHandler::load_image(path, img);
#ifdef CIMG_FOUND
	REQUIRE(success);
	REQUIRE(img.width == 3);
	REQUIRE(img.height == 3);
	REQUIRE(img.maxval == 255);
	REQUIRE(img.pixels.size() == 3 * 3 * 3); // width * height * 3 (RGB)
	REQUIRE(img.pixels == expected_pixels);  // PNG is lossless, so exact match
#else
	REQUIRE(!success);
#endif
}

TEST_CASE("FileHandler JPG Loading", "[FileHandler][JPG]")
{
	imgclean::FilePath path = imgclean::FileHandler::make_file_path("../res/3x3-test.jpg");
	imgclean::PPMImage img;
	bool success = imgclean::FileHandler::load_image(path, img);
#ifdef CIMG_FOUND
	REQUIRE(success);
	REQUIRE(img.width == 3);
	REQUIRE(img.height == 3);
	REQUIRE(img.maxval == 255);
	REQUIRE(img.pixels.size() == 3 * 3 * 3); // width * height * 3 (RGB)

	// JPG is lossy -> need some tolerance in comparison (2 out of 255)
	for (size_t i = 0; i < img.pixels.size(); ++i)
	{
		REQUIRE(std::abs(static_cast<int>(img.pixels[i]) - static_cast<int>(expected_pixels[i])) <= 2);
	}
#else
	REQUIRE(!success);
#endif
}

TEST_CASE("FileHandler PNG Saving", "[FileHandler][PNG]")
{
	// Load from PPM and save to PNG
	imgclean::FilePath load_path = imgclean::FileHandler::make_file_path("../res/3x3-test.ppm");
	imgclean::FilePath save_path = imgclean::FileHandler::make_file_path(
		"../build/test_output/3x3-test-output.png");
	imgclean::PPMImage img;
	bool load_success = imgclean::FileHandler::load_image(load_path, img);
	REQUIRE(load_success);
	bool save_success = imgclean::FileHandler::save_image(save_path, img);
#ifdef CIMG_FOUND
	REQUIRE(save_success);
#else
	REQUIRE(!save_success);
#endif
}

TEST_CASE("FileHandler JPG Saving", "[FileHandler][JPG]")
{
	// Load from PPM and save to JPG
	imgclean::FilePath load_path = imgclean::FileHandler::make_file_path("../res/3x3-test.ppm");
	imgclean::FilePath save_path = imgclean::FileHandler::make_file_path(
		"../build/test_output/3x3-test-output.jpg");
	imgclean::PPMImage img;
	bool load_success = imgclean::FileHandler::load_image(load_path, img);
	REQUIRE(load_success);
	bool save_success = imgclean::FileHandler::save_image(save_path, img);
#ifdef CIMG_FOUND
	REQUIRE(save_success);
#else
	REQUIRE(!save_success);
#endif
}

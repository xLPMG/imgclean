#include "catch.hpp"

#include "imgclean/processors/HelperProcessor.hpp"

TEST_CASE("HelperProcessor rgb_to_linear_grayscale", "[HelperProcessor]")
{
	imgclean::PPMImage rgb_image;
	rgb_image.width  = 3;
	rgb_image.height = 3;
	rgb_image.maxval = 255;
	rgb_image.pixels = {255, 0, 0, 0, 255, 0,   0,   0,   255, 255, 255, 255, 103, 103,
	                    103, 0, 0, 0, 0,   255, 255, 255, 0,   255, 255, 255, 0}; // arbitrary RGB values

	imgclean::GSImage gray_image = imgclean::processors::HelperProcessor::rgb_to_linear_grayscale(rgb_image);

	// dimensions should not change
	REQUIRE(gray_image.width == rgb_image.width);
	REQUIRE(gray_image.height == rgb_image.height);
	REQUIRE(gray_image.maxval == 255);
	// testing the actual pixel values does not make much sense, but we can check that RGB pixels with
	// three values each are reduced to grayscale pixels with one value each
	REQUIRE(gray_image.pixels.size() == rgb_image.width * rgb_image.height); // width * height
}

TEST_CASE("HelperProcessor grayscale_to_rgb", "[HelperProcessor]")
{
	imgclean::GSImage gray_image;
	gray_image.width  = 3;
	gray_image.height = 3;
	gray_image.maxval = 255;
	gray_image.pixels = {76, 150, 29, 255, 103, 0, 0, 255, 255}; // arbitrary grayscale values

	imgclean::PPMImage rgb_image = imgclean::processors::HelperProcessor::grayscale_to_rgb(gray_image);

	// dimensions should not change
	REQUIRE(rgb_image.width == gray_image.width);
	REQUIRE(rgb_image.height == gray_image.height);
	REQUIRE(rgb_image.maxval == 255);
	// testing the actual pixel values does not make much sense, but we can check that grayscale pixels with
	// one value each are expanded to RGB pixels with three values each
	REQUIRE(rgb_image.pixels.size() == gray_image.width * gray_image.height * 3); // width * height * 3 (RGB)
}

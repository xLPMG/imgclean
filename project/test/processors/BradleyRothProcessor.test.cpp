#include "catch.hpp"

#include "imgclean/processors/BradleyRothProcessor.hpp"

TEST_CASE("BradleyRothProcessor apply", "[BradleyRothProcessor]")
{
	imgclean::GSImage gray_image;
	gray_image.width  = 9;
	gray_image.height = 9;
	gray_image.maxval = 255;
	gray_image.pixels = {52, 55, 61, 66, 70,  61,  64,  255, 0,  63, 59,  55,  90, 109, 85,  69,  72,
	                     0,  62, 59, 68, 113, 144, 104, 66,  73, 0,  255, 58,  71, 122, 154, 106, 112,
	                     69, 0,  63, 59, 68,  104, 126, 88,  68, 70, 0,   67,  61, 68,  104, 126, 88,
	                     68, 70, 0,  79, 255, 60,  70,  255, 68, 58, 75,  0,   85, 71,  64,  59,  55,
	                     61, 65, 83, 0,  87,  79,  69,  68,  65, 73, 78,  255, 0}; // arbitrary grayscale values

	imgclean::GSImage processed_image = imgclean::processors::BradleyRothProcessor::apply(gray_image);

	REQUIRE(processed_image.width == gray_image.width);
	REQUIRE(processed_image.height == gray_image.height);
	REQUIRE(processed_image.maxval == 255);
	REQUIRE(processed_image.pixels.size() == gray_image.width * gray_image.height);

    // it is quite difficult and maybe even not practical to test exact pixel values here
    // instead, we only test that the output image is different from the input image
    // to verify that some processing has been done
    REQUIRE(processed_image.pixels != gray_image.pixels);
}

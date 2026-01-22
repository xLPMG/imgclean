#include "catch.hpp"

#include "imgclean/ImgClean.hpp"
#include "imgclean/ImageFormat.hpp"

TEST_CASE("ImgClean clean_image success", "[ImgClean]")
{
    REQUIRE(imgclean::ImgClean::clean_image("res/test/3x3-test.ppm", "test_output/success-test-output.ppm", "Bataineh"));
}

TEST_CASE("ImgClean clean_image file_does_not_exist", "[ImgClean]")
{
    REQUIRE_FALSE(imgclean::ImgClean::clean_image("non_existent_file.ppm", "output.ppm", "Bataineh"));
}

TEST_CASE("ImgClean clean_image unknown_approach", "[ImgClean]")
{
    REQUIRE_FALSE(imgclean::ImgClean::clean_image("res/test/3x3-test.ppm", "test_output/fail-test-output.ppm", "404"));
}
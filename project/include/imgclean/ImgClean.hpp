#ifndef IMGCLEAN_HPP
#define IMGCLEAN_HPP

#include <string>

namespace imgclean
{

enum class ImageFormat;

class ImgClean
{
public:
	//! Check if the required format support is available based on compile-time definitions
	static bool check_format_support(const imgclean::ImageFormat& format, const std::string& path);

	//! Clean the image at input_path and save the result to output_path
	static bool clean_image(const std::string& input_path, const std::string& output_path, const std::string& approach);
};
} // namespace imgclean

#endif

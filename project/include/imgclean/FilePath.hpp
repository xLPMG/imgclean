#ifndef IMGCLEAN_FILEPATH_HPP
#define IMGCLEAN_FILEPATH_HPP

#include "imgclean/ImageFormat.hpp"
#include <string>

namespace imgclean
{

//! File path with associated image format
struct FilePath
{
	std::string path;
	ImageFormat format = ImageFormat::UNKNOWN;
};

} // namespace imgclean

#endif // IMGCLEAN_FILEPATH_HPP

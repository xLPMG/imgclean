#ifndef IMGCLEAN_FILEHANDLER_HPP
#define IMGCLEAN_FILEHANDLER_HPP

#include "imgclean/FilePath.hpp"
#include "imgclean/ImageFormat.hpp"
#include "imgclean/PPMImage.hpp"
#include <string>
#include <vector>

namespace imgclean
{
class FileHandler
{
public:

    //! Convert string to lowercase
    static std::string to_lower(std::string s);

	//! Detect format from file extension (e.g. .ppm, .png, .jpg, .jpeg)
	static ImageFormat detect_format(const std::string& path);

	//! Convenience to build a FilePath with detected format
	static FilePath make_file_path(const std::string& path);

	//! Loads an image into PPM
	//! The file type is inferred from src file ending
	static bool load_image(const FilePath& src, PPMImage& out);

	//! Saves an image from PPM
	//! The file type is inferred from dst file ending
	static bool save_image(const FilePath& dst, const PPMImage& img);
};
} // namespace imgclean

#endif // IMGCLEAN_FILEHANDLER_HPP

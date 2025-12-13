#include "imgclean/FileHandler.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <omp.h>
#include <sstream>

#ifdef CIMG_FOUND
# define cimg_display 0 // we dont need to display images -> reduce dependencies
# include <CImg.h>
#endif

namespace imgclean
{

std::string FileHandler::to_lower(std::string s)
{
	std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
	return s;
}

ImageFormat FileHandler::detect_format(const std::string& path)
{
	auto p   = to_lower(path);
	auto dot = p.find_last_of('.');
	if (dot == std::string::npos) return ImageFormat::UNKNOWN;
	auto ext = p.substr(dot + 1);
	if (ext == "ppm") return ImageFormat::PPM_ASCII;
	if (ext == "png") return ImageFormat::PNG;
	if (ext == "jpg" || ext == "jpeg") return ImageFormat::JPG;
	return ImageFormat::UNKNOWN;
}

FilePath FileHandler::make_file_path(const std::string& path)
{
	return FilePath{path, detect_format(path)};
}

bool FileHandler::load_image(const FilePath& src, PPMImage& out)
{
	if (src.format == ImageFormat::UNKNOWN) return false;

	// Handle PPM_ASCII (P3) format manually
	if (src.format == ImageFormat::PPM_ASCII)
	{
		std::ifstream file(src.path);
		if (!file.is_open()) return false;

		std::string magic;
		file >> magic;
		// only accept P3 (ASCII) format
		if (magic != "P3") return false;

		// Skip comments
		char c = file.peek();
		while (c == '#' || std::isspace(c))
		{
			if (c == '#')
			{
				std::string comment;
				std::getline(file, comment);
			}
			else
			{
				file.get();
			}
			c = file.peek();
		}

		// Read width, height, maxval
		// Check if values are non-negative
		if (!(file >> out.width >> out.height >> out.maxval) || out.width <= 0 || out.height <= 0 ||
		    out.maxval <= 0)
		{
			out.clear();
			return false;
		}

		// Read pixel data
		size_t pixel_count = static_cast<size_t>(out.width) * out.height * 3;
		out.pixels.resize(pixel_count);

		for (size_t i = 0; i < pixel_count; ++i)
		{
			int val;
			if (!(file >> val))
			{
				out.clear();
				return false;
			}
			out.pixels[i] = static_cast<uint16_t>(val);
		}

		return true;
	}

#ifdef CIMG_FOUND
	// Handle PNG and JPG formats using CImg
	try
	{
		cimg_library::CImg<unsigned char> img(src.path.c_str());

		out.width          = img.width();
		out.height         = img.height();
		out.maxval         = 255;
		size_t pixel_count = static_cast<size_t>(out.width) * out.height * 3;
		out.pixels.resize(pixel_count);

#pragma omp parallel for collapse(2)
		for (int y = 0; y < out.height; ++y)
		{
			for (int x = 0; x < out.width; ++x)
			{
				// in PPM, pixels are stored like R,G,B,R,G,B,...
				// Access in img: x, y, depth, channel
				size_t idx          = (y * out.width + x) * 3;
				out.pixels[idx + 0] = img(x, y, 0, 0); // R
				out.pixels[idx + 1] = img(x, y, 0, 1); // G
				out.pixels[idx + 2] = img(x, y, 0, 2); // B
			}
		}

		return true;
	}
	catch (const cimg_library::CImgException&)
	{
		return false;
	}
#else
	return false;
#endif
}

bool FileHandler::save_image(const FilePath& dst, const PPMImage& img)
{
	if (dst.format == ImageFormat::UNKNOWN) return false;

	// create output directory if it doesnt exist
	std::filesystem::path file_path(dst.path);
	if (file_path.has_parent_path())
	{
		std::error_code ec;
		std::filesystem::create_directories(file_path.parent_path(), ec);
		if (ec) return false;
	}

	// Handle PPM_ASCII (P3) format manually
	if (dst.format == ImageFormat::PPM_ASCII)
	{
		std::ofstream file(dst.path);
		if (!file.is_open()) return false;

		// Write PPM header
		file << "P3\n";
		file << img.width << " " << img.height << "\n";
		file << img.maxval << "\n";

		// Write pixel data
		size_t pixel_count = static_cast<size_t>(img.width) * img.height * 3;
		for (size_t i = 0; i < pixel_count; ++i)
		{
			file << img.pixels[i];
			if ((i + 1) % 3 == 0)
			{
				file << "\n"; // New line after each pixel
			}
			else
			{
				file << " ";
			}
		}

		return file.good();
	}

#ifdef CIMG_FOUND
	// Handle PNG and JPG formats using CImg
	try
	{
		cimg_library::CImg<unsigned char> cimg(img.width, img.height, 1, 3);

#pragma omp parallel for collapse(2)
		for (int y = 0; y < img.height; ++y)
		{
			for (int x = 0; x < img.width; ++x)
			{
				// in PPM, pixels are stored like R,G,B,R,G,B,...
				// Access in img: x, y, depth, channel
				size_t idx       = (y * img.width + x) * 3;
				cimg(x, y, 0, 0) = static_cast<unsigned char>(img.pixels[idx + 0]); // R
				cimg(x, y, 0, 1) = static_cast<unsigned char>(img.pixels[idx + 1]); // G
				cimg(x, y, 0, 2) = static_cast<unsigned char>(img.pixels[idx + 2]); // B
			}
		}

		cimg.save(dst.path.c_str());
		return true;
	}
	catch (const cimg_library::CImgException&)
	{
		return false;
	}
#else
	return false;
#endif
}

} // namespace imgclean

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

ImageFormat FileHandler::detect_format(const std::string& path)
{
	auto dot = path.find_last_of('.');
	if (dot == std::string::npos) return ImageFormat::UNKNOWN;
	std::string ext = path.substr(dot + 1);
	std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
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
		std::ifstream file(src.path, std::ios::binary);
		if (!file) return false;

		// ppm header
		std::string line;
		size_t values_read = 0;
		int header[3]      = {0, 0, 0}; // width, height, maxval

		std::getline(file, line);
		if (line != "P3") return false; // first line must be P3
		while (values_read < 3 && std::getline(file, line))
		{
			if (line.empty() || line[0] == '#') continue;

			const char* p = line.c_str();
			char* end;

			while (*p && values_read < 3)
			{
				long v = std::strtol(p, &end, 10);
				if (p == end) break;
				header[values_read++] = static_cast<int>(v);
				p                     = end;
			}
		}

		if (values_read != 3)
		{
			out.clear();
			return false;
		}

		out.width  = header[0];
		out.height = header[1];
		out.maxval = header[2];

		const size_t pixel_count = static_cast<size_t>(out.width) * static_cast<size_t>(out.height) * 3;
		out.pixels.resize(pixel_count);

		// read pixel data
		size_t idx    = 0;
		uint16_t* dst = out.pixels.data();
		while (idx < pixel_count && std::getline(file, line))
		{
			if (line.empty() || line[0] == '#') continue;

			const char* p = line.c_str();
			char* end     = nullptr;

			while (*p && idx < pixel_count)
			{
				long v = std::strtol(p, &end, 10);
				if (p == end) break;

				if (v < 0 || v > out.maxval)
				{
					out.clear();
					return false;
				}

				dst[idx++] = static_cast<uint16_t>(v);
				p          = end;
			}
		}

		if (idx != pixel_count)
		{
			out.clear();
			return false;
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
		const size_t pixel_count = static_cast<size_t>(out.width) * static_cast<size_t>(out.height) * 3u;
		out.pixels.resize(pixel_count);

# pragma omp parallel for collapse(2)
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
		const auto parent = file_path.parent_path();
		if (!std::filesystem::exists(parent))
		{
			std::filesystem::create_directories(parent, ec);
		}
		if (ec) return false;
	}

	// Handle PPM_ASCII (P3) format manually
	if (dst.format == ImageFormat::PPM_ASCII)
	{
		std::ofstream file(dst.path);
		if (!file.is_open()) return false;

		const size_t pixel_count = static_cast<size_t>(img.width) * img.height * 3;

		std::string buffer;
		buffer.reserve(1 << 20);
		buffer += "P3\n";
		buffer += std::to_string(img.width) + " " + std::to_string(img.height) + "\n";
		buffer += std::to_string(img.maxval) + "\n";
		for (size_t i = 0; i < pixel_count; ++i)
		{
			buffer += std::to_string(img.pixels[i]);
			buffer += ((i + 1) % 3 == 0) ? '\n' : ' ';
			if (buffer.size() > (1 << 19))
			{
				file.write(buffer.data(), buffer.size());
				buffer.clear();
			}
		}

		if (!buffer.empty()) file.write(buffer.data(), buffer.size());

		return file.good();
	}

#ifdef CIMG_FOUND
	// Handle PNG and JPG formats using CImg
	try
	{
		cimg_library::CImg<unsigned char> cimg(img.width, img.height, 1, 3);

# pragma omp parallel for collapse(2)
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

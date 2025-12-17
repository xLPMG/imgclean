#include "imgclean/FileHandler.hpp"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <vector>
#include <cstring>
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
		// get file size
		const auto sz = std::filesystem::file_size(src.path);
		if (sz == 0) return false;

		// create buffer
		std::string buf;
		buf.resize(sz);

		// read file into buffer
		std::ifstream file(src.path, std::ios::binary);
		if (!file.read(buf.data(), sz)) return false;

		const char* p   = buf.data();
		const char* end = buf.data() + buf.size();

		//! Helper to skip whitespace and comments
		auto skip_ws_and_comments = [](const char*& it, const char* it_end)
		{
			while (it < it_end)
			{
				// Skip whitespace
				while (it < it_end && static_cast<unsigned char>(*it) <= ' ')
					++it;
				if (it >= it_end) break;
				if (*it == '#')
				{
					// Skip comment until end-of-line
					while (it < it_end && *it != '\n' && *it != '\r')
						++it;
					continue;
				}
				break;
			}
		};

		//! Helper to parse an integer
		auto parse_int = [](const char*& it, const char* it_end, int& out_val) -> bool
		{
			std::from_chars_result res{nullptr, std::errc{}};
			// Use from_chars on the remaining range
			res = std::from_chars(it, it_end, out_val, 10);
			if (res.ptr == it)
			{
				return false;
			}
			it = res.ptr;
			return res.ec == std::errc{};
		};

		// Parse magic number "P3"
		skip_ws_and_comments(p, end);
		if (p >= end || *p != 'P') return false;
		++p;
		if (p >= end || *p != '3') return false;
		++p;

		// Parse width, height, maxval
		skip_ws_and_comments(p, end);
		int header_w = 0;
		if (!parse_int(p, end, header_w)) return false;

		skip_ws_and_comments(p, end);
		int header_h = 0;
		if (!parse_int(p, end, header_h)) return false;

		skip_ws_and_comments(p, end);
		int header_max = 0;
		if (!parse_int(p, end, header_max)) return false;

		if (header_w <= 0 || header_h <= 0 || header_max <= 0) return false;

		out.width  = header_w;
		out.height = header_h;
		out.maxval = header_max;

		const size_t pixel_count = static_cast<size_t>(out.width) * static_cast<size_t>(out.height) * 3u;
		out.pixels.resize(pixel_count);

		// Parse pixel data
		size_t idx    = 0;
		uint16_t* dst = out.pixels.data();

		while (idx < pixel_count)
		{
			skip_ws_and_comments(p, end);
			if (p >= end) break;

			int value = 0;
			if (!parse_int(p, end, value)) break;
			if (value < 0 || value > out.maxval || value > 65535)
			{
				out.clear();
				return false;
			}
			dst[idx++] = static_cast<uint16_t>(value);
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
		out.width                = img.width();
		out.height               = img.height();
		out.maxval               = 255;
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
		std::ofstream file(dst.path, std::ios::binary);
		if (!file.is_open()) return false;

		// 1 MiB buffer
		constexpr size_t kBufCap = 1u << 20;
		std::vector<char> buf;
		buf.resize(kBufCap);
		size_t pos = 0;

		//! Helper to flush buffer to file
		auto flush = [&]() {
			if (pos) { file.write(buf.data(), static_cast<std::streamsize>(pos)); pos = 0; }
		};

		//! Helper to ensure there is enough space in buffer, flushing if necessary
		auto ensure = [&](size_t need) {
			if (pos + need > buf.size()) flush();
		};

		//! Helpers to push data into buffer
		auto push_char = [&](char c) {
			ensure(1);
			buf[pos++] = c;
		};

		//! Helper to push multiple chars into buffer
		auto push_chars = [&](const char* p, size_t n) {
			if (n > buf.size()) { // extremely unlikely, but be safe
				flush();
				file.write(p, static_cast<std::streamsize>(n));
				return;
			}
			if (pos + n > buf.size()) flush();
			std::memcpy(buf.data() + pos, p, n);
			pos += n;
		};

		//! Helper to push an integer into buffer
		auto push_int = [&](uint16_t v) {
			char tmp[6]; // max 5 digits for 65535 + NUL
			auto res = std::to_chars(tmp, tmp + sizeof(tmp), v, 10);
			push_chars(tmp, static_cast<size_t>(res.ptr - tmp));
		};

		// Header: P3\n<width> <height>\n<maxval>\n
		push_chars("P3\n", 3);
		push_int(static_cast<uint16_t>(img.width));
		push_char(' ');
		push_int(static_cast<uint16_t>(img.height));
		push_char('\n');
		push_int(static_cast<uint16_t>(img.maxval));
		push_char('\n');

		// Body: each pixel as "R G B\n"
		const uint16_t* px = img.pixels.data();
		const size_t pixel_count = static_cast<size_t>(img.width) * static_cast<size_t>(img.height) * 3u;
		for (size_t i = 0; i < pixel_count; i += 3)
		{
			push_int(px[i + 0]);
			push_char(' ');
			push_int(px[i + 1]);
			push_char(' ');
			push_int(px[i + 2]);
			push_char('\n');
		}

		flush();
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

#ifndef IMGCLEAN_GSIMAGE_HPP
#define IMGCLEAN_GSIMAGE_HPP

#include <cstdint>
#include <vector>

namespace imgclean
{

//! Struct to store an 8-bit gray scale image
struct GSImage
{
	int width  = 0;
	int height = 0;
	int maxval = 255;
	std::vector<uint8_t> pixels; // size = width * height

	bool empty() const { return width <= 0 || height <= 0 || pixels.empty(); }
	void clear()
	{
		pixels.clear();
		width  = 0;
		height = 0;
		maxval = 255;
	}
};

} // namespace imgclean

#endif // IMGCLEAN_GSIMAGE_HPP

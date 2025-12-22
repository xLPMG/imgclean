#ifndef IMGCLEAN_PPMIMAGE_HPP
#define IMGCLEAN_PPMIMAGE_HPP

#include <cstdint>
#include <vector>

namespace imgclean
{

//! Struct to store an ASCII PPM (P3) image
struct PPMImage
{
	int width  = 0;
	int height = 0;
	int maxval = 255;
	std::vector<uint16_t> pixels; // size = width * height * 3 (RGB)
	std::vector<unsigned char> exif_data; // EXIF segment for JPG

	bool empty() const { return width <= 0 || height <= 0 || pixels.empty(); }
	void clear()
	{
		pixels.clear();
		exif_data.clear();
		width  = 0;
		height = 0;
		maxval = 255;
	}
};

} // namespace imgclean

#endif // IMGCLEAN_PPMIMAGE_HPP

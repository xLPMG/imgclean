#include "imgclean/processors/IntegralImageProcessor.hpp"

#include <cstdio>
#include <vector>

namespace imgclean
{
namespace processors
{
GSImage IntegralImageProcessor::apply(const GSImage& image)
{
	if (image.empty()) return GSImage();

	// compute the integral image
	std::vector<uint32_t> integral(image.width * image.height, 0);
	for (int y = 0; y < image.height; ++y)
	{
		for (int x = 0; x < image.width; ++x)
		{
			int idx             = y * image.width + x;
			uint32_t left       = (x > 0) ? integral[y * image.width + (x - 1)] : 0;
			uint32_t above      = (y > 0) ? integral[(y - 1) * image.width + x] : 0;
			uint32_t above_left = (y > 0 && x > 0) ? integral[(y - 1) * image.width + (x - 1)] : 0;

			integral[y * image.width + x] = static_cast<uint32_t>(image.pixels[idx]) + left + above -
			                                above_left;
		}
	}

	GSImage output_image;
	output_image.width  = image.width;
	output_image.height = image.height;
	output_image.maxval = image.maxval;
	output_image.exif_data = image.exif_data;
	output_image.pixels.resize(image.width * image.height);

	for (int i = 0; i < image.width; ++i)
	{
		for (int j = 0; j < image.height; ++j)
		{
			int x1     = std::max(0, i - half_window);
			int y1     = std::max(0, j - half_window);
			int x2     = std::min(image.width - 1, i + half_window);
			int y2     = std::min(image.height - 1, j + half_window);
			int count  = (x2 - x1 + 1) * (y2 - y1 + 1);
			uint32_t A = integral[y2 * image.width + x2];
			uint32_t B = (y1 > 0) ? integral[(y1 - 1) * image.width + x2] : 0;
			uint32_t C = (x1 > 0) ? integral[y2 * image.width + (x1 - 1)] : 0;
			uint32_t D = (y1 > 0 && x1 > 0) ? integral[(y1 - 1) * image.width + (x1 - 1)] : 0;

			float local_mean = static_cast<float>(A - B - C + D) / count;
			float pixel_val  = static_cast<float>(image.pixels[j * image.width + i]);

			output_image.pixels[j * image.width + i] = (pixel_val < t * local_mean) ? 0 : 255;
		}
	}

	return output_image;
}

} // namespace processors
} // namespace imgclean

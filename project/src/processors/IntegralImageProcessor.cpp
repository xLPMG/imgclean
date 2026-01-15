#include "imgclean/processors/IntegralImageProcessor.hpp"

#include <vector>
#include <algorithm> // std::max, std::min

namespace imgclean
{
namespace processors
{
/**
 * @brief Applies integral image based binarization to the input grayscale image.
 */
GSImage IntegralImageProcessor::apply(const GSImage& image)
{
	// Note: This implementation seems to be so fast already, that using any OpenMP pragmas
	// on the for-loops actually made it slower.
	if (image.empty()) return GSImage();

	const int image_width  = image.width;
	const int image_height = image.height;

	// compute the integral image
	std::vector<uint32_t> integral(image_width * image_height, 0);

	for (int y = 0; y < image_height; ++y)
	{
		for (int x = 0; x < image_width; ++x)
		{
			const size_t index        = y * image_width + x;
			const uint32_t left       = (x > 0) ? integral[y * image_width + (x - 1)] : 0;
			const uint32_t above      = (y > 0) ? integral[(y - 1) * image_width + x] : 0;
			const uint32_t above_left = (y > 0 && x > 0) ? integral[(y - 1) * image_width + (x - 1)] : 0;

			integral[y * image_width + x] = static_cast<uint32_t>(image.pixels[index]) + left + above -
			                                above_left;
		}
	}

	GSImage output_image;
	output_image.width     = image_width;
	output_image.height    = image_height;
	output_image.maxval    = image.maxval;
	output_image.exif_data = image.exif_data;
	output_image.pixels.resize(image_width * image_height);

	for (int j = 0; j < image_height; ++j)
	{
		for (int i = 0; i < image_width; ++i)
		{
			const int x1     = std::max(0, i - half_window);
			const int y1     = std::max(0, j - half_window);
			const int x2     = std::min(image_width - 1, i + half_window);
			const int y2     = std::min(image_height - 1, j + half_window);
			const int count  = (x2 - x1 + 1) * (y2 - y1 + 1);
			const uint32_t A = integral[y2 * image_width + x2];
			const uint32_t B = (y1 > 0) ? integral[(y1 - 1) * image_width + x2] : 0;
			const uint32_t C = (x1 > 0) ? integral[y2 * image_width + (x1 - 1)] : 0;
			const uint32_t D = (y1 > 0 && x1 > 0) ? integral[(y1 - 1) * image_width + (x1 - 1)] : 0;

			const float local_mean = static_cast<float>(A - B - C + D) / count;
			const float pixel_val  = static_cast<float>(image.pixels[j * image_width + i]);
			output_image.pixels[j * image.width + i] = (pixel_val < t * local_mean) ? 0 : 255;
		}
	}

	return output_image;
}

} // namespace processors
} // namespace imgclean

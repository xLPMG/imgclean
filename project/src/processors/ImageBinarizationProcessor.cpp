#include "imgclean/processors/ImageBinarizationProcessor.hpp"

#include <cmath>
#include <limits>
#include <numeric>

namespace imgclean::processors
{

GSImage ImageBinarizationProcessor::apply(const GSImage& image)
{
	if (image.empty()) return GSImage();

	const int width             = image.width;
	const int height            = image.height;
	const size_t num_pixels     = image.pixels.size();
	const unsigned char* pixels = image.pixels.data();

	std::vector<float> windows_mean(num_pixels);
	std::vector<float> windows_stddev(num_pixels);
	float w_min_stddev = std::numeric_limits<float>::max();
	float w_max_stddev = std::numeric_limits<float>::min();

	for (int j = 0; j < height; ++j)
	{
		for (int i = 0; i < width; ++i)
		{
			int x1 = std::max(0, i - half_window);
			int y1 = std::max(0, j - half_window);
			int x2 = std::min(width - 1, i + half_window);
			int y2 = std::min(height - 1, j + half_window);

			float tmp_acc = 0;
			for (int y = y1; y <= y2; ++y)
			{
				for (int x = x1; x <= x2; ++x)
				{
					tmp_acc += pixels[y * width + x];
				}
			}
			float cur_mean              = tmp_acc / ((x2 - x1 + 1) * (y2 - y1 + 1));
			windows_mean[j * width + i] = cur_mean;

			float cur_stddev = 0.0;
			for (int y = y1; y <= y2; ++y)
			{
				for (int x = x1; x <= x2; ++x)
				{
					float diff = pixels[y * width + x] - cur_mean;
					cur_stddev += diff * diff;
				}
			}

			cur_stddev = std::sqrt(cur_stddev / ((x2 - x1 + 1) * (y2 - y1 + 1)));

			if (cur_stddev > w_max_stddev)
			{
				w_max_stddev = cur_stddev;
			}

			if (cur_stddev < w_min_stddev)
			{
				w_min_stddev = cur_stddev;
			}

			windows_stddev[j * width + i] = cur_stddev;
		}
	}

	// at this point: g_mean, w_min;max_stddev

	// prepare output
	GSImage output_image;
	output_image.width     = width;
	output_image.height    = height;
	output_image.maxval    = image.maxval;
	output_image.exif_data = image.exif_data;
	output_image.pixels.resize(num_pixels);

	const float global_mean = std::accumulate(pixels, pixels + num_pixels, 0.0f) / static_cast<float>(num_pixels);

	// Iterate again for binarization
	for (int j = 0; j < height; ++j)
	{
		for (int i = 0; i < width; ++i)
		{
			const size_t index        = j * width + i;
			const float current_value = windows_stddev[index];

			// adaptive_stddev
			float adaptive_stddev = 0.0;
			if (w_max_stddev > w_min_stddev)
			{
				adaptive_stddev = (current_value - w_min_stddev) / (w_max_stddev - w_min_stddev);
			}

			// threshold calculation
			float threshold = current_value -
			                  (windows_mean[index] * windows_mean[index] - current_value) /
			                          ((global_mean + current_value) * (adaptive_stddev + current_value));

			// binarization
			output_image.pixels[index] = (pixels[index] < threshold) ? 0 : 255;
		}
	}

	return output_image;
}

} // namespace imgclean::processors

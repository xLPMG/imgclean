/*
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

	std::vector<float> windows_mean(num_pixels, 0);
	std::vector<float> windows_stddev(num_pixels, 0);
	float w_min_stddev = std::numeric_limits<float>::max();
	float w_max_stddev = std::numeric_limits<float>::min();

	for (int j = 0; j < height; ++j)
	{
		for (int i = 0; i < width; ++i)
		{
			const int x1 = std::max(0, i - half_window);
			const int y1 = std::max(0, j - half_window);
			const int x2 = std::min(width - 1, i + half_window);
			const int y2 = std::min(height - 1, j + half_window);

			float sum    = 0.0f;
			float sum_sq = 0.0f;

			for (int y = y1; y <= y2; ++y)
			{
				const int row_offset = y * width;
				for (int x = x1; x <= x2; ++x)
				{
					const float val = static_cast<float>(pixels[row_offset + x]);
					sum += val;
					sum_sq += val * val;
				}
			}
			const float window_size = static_cast<float>((x2 - x1 + 1) * (y2 - y1 + 1));

			const float cur_mean = sum / window_size;

			float variance = sum_sq / window_size - cur_mean * cur_mean;

			variance               = std::max(variance, 0.0f);
			const float cur_stddev = std::sqrt(variance);

			// store results
			const size_t index    = j * width + i;
			windows_mean[index]   = cur_mean;
			windows_stddev[index] = cur_stddev;

			w_max_stddev = std::max(w_max_stddev, cur_stddev);
			w_min_stddev = std::min(w_min_stddev, cur_stddev);
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
			const size_t index         = j * width + i;
			const float current_stddev = windows_stddev[index];
			const float current_mean   = windows_mean[index];

			// adaptive_stddev
			float adaptive_stddev = 0.0;
			if (w_max_stddev > w_min_stddev)
			{
				adaptive_stddev = (current_stddev - w_min_stddev) / (w_max_stddev - w_min_stddev);
			}

			// threshold calculation
			float threshold = current_mean -
			                  (current_mean * current_mean - current_stddev) /
			                          ((global_mean + current_stddev) * (adaptive_stddev + current_stddev));

			// binarization
			output_image.pixels[index] = (pixels[index] < threshold) ? 0 : 255;
		}
	}

	return output_image;
}

} // namespace imgclean::processors
*/
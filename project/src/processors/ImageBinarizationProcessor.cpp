#include "imgclean/processors/ImageBinarizationProcessor.hpp"

#include <cmath>
#include <limits>
#include <numeric>

namespace imgclean::processors
{

GSImage ImageBinarizationProcessor::apply(const GSImage &image)
{
	if (image.empty()) return GSImage();

	// calculate mean
	float g_mean = 0;
	for (uint8_t pixel : image.pixels)
	{
		g_mean += pixel;
	}
	g_mean /= image.pixels.size();

	// calculate min, max window
	std::vector<float> windows_mean(image.pixels.size());
	std::vector<float> windows_stddev(image.pixels.size());
	float w_min_stddev = std::numeric_limits<float>::max();
	float w_max_stddev = std::numeric_limits<float>::min();

	for (int i = 0; i < image.width; ++i)
	{
		for (int j = 0; j < image.height; ++j)
		{
			int x1 = std::max(0, i - half_window);
			int y1 = std::max(0, j - half_window);
			int x2 = std::min(image.width - 1, i + half_window);
			int y2 = std::min(image.height - 1, j + half_window);

			float tmp_acc = 0;
			for ( int x = x1; x <= x2; ++x)
			{
				for ( int y = y1; y <= y2; ++y)
				{
					tmp_acc += image.pixels[y * image.width + x];
				}
			}
			float cur_mean = tmp_acc / ((x2 - x1 + 1) * (y2 - y1 + 1));
			windows_mean[j * image.width + i] = cur_mean;

			float cur_stddev = 0.0;
			for (int x = x1; x <= x2; ++x)
			{
				for ( int y = y1; y <= y2; ++y)
				{
					cur_stddev += (image.pixels[y * image.width + x] - cur_mean) * (image.pixels[y * image.width + x] - cur_mean);
				}
			}

			cur_stddev = std::sqrt(cur_stddev / ((x2 - x1 + 1) * (y2 - y1 + 1)));

			if ( cur_stddev > w_max_stddev )
			{
				w_max_stddev = cur_stddev;
			}

			if ( cur_stddev < w_min_stddev )
			{
				w_min_stddev = cur_stddev;
			}

			windows_stddev[j * image.width + i] = cur_stddev;
		}
	}

	// at this point: g_mean, w_min;max_stddev

	// pepare output
	GSImage output_image;
	output_image.width  = image.width;
	output_image.height = image.height;
	output_image.maxval = image.maxval;
	output_image.exif_data = image.exif_data;
	output_image.pixels.resize(image.width * image.height);

	// NOW: iterate again
	for (int i = 0; i < image.width; ++i)
	{
		for (int j = 0; j < image.height; ++j)
		{
			// adaptive_stddev
			float adaptive_stddev = 0.0;
			if (w_max_stddev > w_min_stddev)
			{
				adaptive_stddev = (windows_stddev[j * image.width + i] - w_min_stddev) / (w_max_stddev - w_min_stddev);
			}

			// threshold calculation
			float threshold = windows_mean[j * image.width + i] - (windows_mean[j * image.width + i] * windows_mean[j * image.width + i ] - windows_stddev[j * image.width + i]) / ((g_mean + windows_stddev[j * image.width + i]) * (adaptive_stddev + windows_stddev[j * image.width + i]));

			// binarization
			output_image.pixels[j * image.width + i] = (image.pixels[j * image.width + i] < threshold) ? 0 : 255;
		}
	}

	return output_image;
}

} // namespace imgclean::processors


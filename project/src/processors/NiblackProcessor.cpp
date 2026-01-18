#include "imgclean/processors/NiblackProcessor.hpp"
#include "cmath"

namespace imgclean::processors
{
/**
 * @brief Applies Niblacks thresholding to an image.
 */
GSImage NiblackProcessor::apply(const GSImage& image)
{
	if (image.empty()) return GSImage();

	const int image_width        = image.width;
	const int image_height       = image.height;
	const std::size_t image_size = image.pixels.size();
	const unsigned char* pixels  = image.pixels.data();

	// Step 1: Compute two Integral Images
	std::vector<uint32_t> sum_table(image_size);
	std::vector<double> sq_sum_table(image_size);

	for (int j = 0; j < image_height; ++j)
	{
		uint32_t row_sum       = 0;
		double row_squared_sum = 0;
		for (int i = 0; i < image_width; ++i)
		{
			const std::size_t idx = j * image_width + i;
			const uint32_t val    = pixels[idx];
			row_sum += val;
			row_squared_sum += static_cast<double>(val) * val;

			if (j == 0)
			{
				sum_table[idx]    = row_sum;
				sq_sum_table[idx] = row_squared_sum;
			}
			else
			{
				sum_table[idx]    = row_sum + sum_table[(j - 1) * image_width + i];
				sq_sum_table[idx] = row_squared_sum + sq_sum_table[(j - 1) * image_width + i];
			}
		}
	}

	// Step 2: Prepare Output-Image & apply thresholding
	GSImage output = image;
	output.pixels.resize(image_size);

	// Constant 'k' for Niblack
	const float k = -0.2f;

#pragma omp parallel for schedule(static)
	for (int j = 0; j < image_height; ++j)
	{
		for (int i = 0; i < image_width; ++i)
		{
			// Window boundaries
			int x1 = std::max(0, i - half_window), x2 = std::min(image_width - 1, i + half_window);
			int y1 = std::max(0, j - half_window), y2 = std::min(image_height - 1, j + half_window);
			const float area = static_cast<float>((x2 - x1 + 1) * (y2 - y1 + 1));

			// Lambda Optimization: for faster sum retrieval
			auto get_area_sum = [&](const auto& table)
			{
				double res = table[y2 * image_width + x2];
				if (x1 > 0) res -= table[y2 * image_width + (x1 - 1)];
				if (y1 > 0) res -= table[(y1 - 1) * image_width + x2];
				if (x1 > 0 && y1 > 0) res += table[(y1 - 1) * image_width + (x1 - 1)];
				return res;
			};

			// Calculate mean (m) and standard deviation (s)
			const float mean         = static_cast<float>(get_area_sum(sum_table) / area);
			const float mean_squared = static_cast<float>(get_area_sum(sq_sum_table) / area);
			const float stddev       = std::sqrt(std::max(0.0f, mean_squared - (mean * mean)));

			// Niblack Threshold
			const float threshold = mean + (k * stddev);

			output.pixels[j * image_width + i] = (pixels[j * image_width + i] < threshold) ? 0 : 255;
		}
	}

	return output;
}
} // namespace imgclean::processors

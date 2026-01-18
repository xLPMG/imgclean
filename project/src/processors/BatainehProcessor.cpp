#include "imgclean/processors/BatainehProcessor.hpp"

#include <cmath>

namespace imgclean::processors
{

GSImage BatainehProcessor::apply(const GSImage& image)
{
	if (image.empty()) return {};

	const int image_width        = image.width;
	const int image_height       = image.height;
	const std::size_t num_pixels = image.pixels.size();
	const unsigned char* pixels  = image.pixels.data();

	// 1. Pre-calculate Integral Images once (Linear scan)
	std::vector<uint32_t> intSum(num_pixels);
	std::vector<float> intSqSum(num_pixels);

	for (int j = 0; j < image_height; ++j)
	{
		uint32_t row_sum             = 0;
		float row_squared_sum        = 0.0f;
		const std::size_t row_offset = static_cast<std::size_t>(j) * image_width;
		for (int i = 0; i < image_width; ++i)
		{
			const std::size_t idx = row_offset + i;
			const auto val        = static_cast<float>(pixels[idx]);
			row_sum += static_cast<uint32_t>(val);
			row_squared_sum += val * val;
			intSum[idx]   = (j == 0) ? row_sum : row_sum + intSum[idx - image_width];
			intSqSum[idx] = (j == 0) ? row_squared_sum : row_squared_sum + intSqSum[idx - image_width];
		}
	}

	const float global_mean = static_cast<float>(intSum.back()) / num_pixels;

	// 2. Fast Global Min/Max StdDev (Sub-sampled to stay in Cache)
	float w_min_stddev = 1000.0f;
	float w_max_stddev = 0.0f;

	// #pragma omp parallel for reduction(min : w_min_stddev) reduction(max : w_max_stddev)
	for (int j = 0; j < image_height; j += 4)
	{
		for (int i = 0; i < image_width; i += 4)
		{
			int x1 = std::max(0, i - half_window), x2 = std::min(image_width - 1, i + half_window);
			int y1 = std::max(0, j - half_window), y2 = std::min(image_height - 1, j + half_window);
			const auto area = static_cast<float>((x2 - x1 + 1) * (y2 - y1 + 1));

			// Lambda Optimization: faster processing
			auto get_v = [&](const auto& t)
			{
				double res = t[y2 * image_width + x2];
				if (x1 > 0) res -= t[y2 * image_width + (x1 - 1)];
				if (y1 > 0) res -= t[(y1 - 1) * image_width + x2];
				if (x1 > 0 && y1 > 0) res += t[(y1 - 1) * image_width + (x1 - 1)];
				return static_cast<float>(res);
			};

			float m = get_v(intSum) / area;
			float s = std::sqrt(std::max(0.0f, (get_v(intSqSum) / area) - (m * m)));
			if (s < w_min_stddev) w_min_stddev = s;
			if (s > w_max_stddev) w_max_stddev = s;
		}
	}

	// 3. Fused Binarization with Static Scheduling
	GSImage output_image = image;
	output_image.pixels.resize(num_pixels);
	const float range = w_max_stddev - w_min_stddev + 0.0001f;

	// #pragma omp parallel for schedule(static)
	for (int j = 0; j < image_height; ++j)
	{
		const std::size_t row_ptr = static_cast<std::size_t>(j) * image_width;
		for (int i = 0; i < image_width; ++i)
		{
			int x1 = std::max(0, i - half_window), x2 = std::min(image_width - 1, i + half_window);
			int y1 = std::max(0, j - half_window), y2 = std::min(image_height - 1, j + half_window);
			const auto area = static_cast<float>((x2 - x1 + 1) * (y2 - y1 + 1));

			auto sum_at = [&](const auto& t)
			{
				double res = t[y2 * image_width + x2];
				if (x1 > 0) res -= t[y2 * image_width + (x1 - 1)];
				if (y1 > 0) res -= t[(y1 - 1) * image_width + x2];
				if (x1 > 0 && y1 > 0) res += t[(y1 - 1) * image_width + (x1 - 1)];
				return static_cast<float>(res);
			};

			const float mean   = sum_at(intSum) / area;
			const float stddev = std::sqrt(std::max(0.0f, (sum_at(intSqSum) / area) - (mean * mean)));
			const float adaptive_stddev = (stddev - w_min_stddev) / range;

			// For formatting
			const float mean_variance_diff = (mean * mean - stddev);
			const float scale_factor       = (global_mean + stddev) * (adaptive_stddev + stddev) + 0.0001f;

			const float threshold = mean - mean_variance_diff / scale_factor;

			output_image.pixels[row_ptr + i] = (pixels[row_ptr + i] < threshold) ? 0 : 255;
		}
	}

	return output_image;
}

} // namespace imgclean::processors

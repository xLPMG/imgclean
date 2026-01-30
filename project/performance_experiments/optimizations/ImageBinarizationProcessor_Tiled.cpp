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

    const int width = image.width;
    const int height = image.height;
    const size_t num_pixels = image.pixels.size();
    const unsigned char* pixels = image.pixels.data();

    // 1. Pre-calculate Integral Images once (Linear scan)
    // We use uint32 and float to keep these as slim as possible
    std::vector<uint32_t> intSum(num_pixels);
    std::vector<float> intSqSum(num_pixels);

    for (int j = 0; j < height; ++j) {
        uint32_t rowSum = 0;
        float rowSqSum = 0.0f;
        const size_t row_offset = (size_t)j * width;
        for (int i = 0; i < width; ++i) {
            const size_t idx = row_offset + i;
            const float val = static_cast<float>(pixels[idx]);
            rowSum += (uint32_t)val;
            rowSqSum += val * val;
            intSum[idx] = (j == 0) ? rowSum : rowSum + intSum[idx - width];
            intSqSum[idx] = (j == 0) ? rowSqSum : rowSqSum + intSqSum[idx - width];
        }
    }

    const float global_mean = (float)intSum.back() / num_pixels;

    // 2. Fast Global Min/Max StdDev (Sub-sampled to stay in Cache)
    float w_min_stddev = 1000.0f, w_max_stddev = 0.0f;
    #pragma omp parallel for reduction(min:w_min_stddev) reduction(max:w_max_stddev)
    for (int j = 0; j < height; j += 4) {
        for (int i = 0; i < width; i += 4) {
            int x1 = std::max(0, i - half_window), x2 = std::min(width - 1, i + half_window);
            int y1 = std::max(0, j - half_window), y2 = std::min(height - 1, j + half_window);
            float area = (float)((x2 - x1 + 1) * (y2 - y1 + 1));

            auto get_v = [&](const auto& t) {
                double res = t[y2 * width + x2];
                if (x1 > 0) res -= t[y2 * width + (x1 - 1)];
                if (y1 > 0) res -= t[(y1 - 1) * width + x2];
                if (x1 > 0 && y1 > 0) res += t[(y1 - 1) * width + (x1 - 1)];
                return (float)res;
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

    #pragma omp parallel for schedule(static)
    for (int j = 0; j < height; ++j) {
        const size_t row_ptr = (size_t)j * width;
        for (int i = 0; i < width; ++i) {
            int x1 = std::max(0, i - half_window), x2 = std::min(width - 1, i + half_window);
            int y1 = std::max(0, j - half_window), y2 = std::min(height - 1, j + half_window);
            float area = (float)((x2 - x1 + 1) * (y2 - y1 + 1));

            auto sum_at = [&](const auto& t) {
                double res = t[y2 * width + x2];
                if (x1 > 0) res -= t[y2 * width + (x1 - 1)];
                if (y1 > 0) res -= t[(y1 - 1) * width + x2];
                if (x1 > 0 && y1 > 0) res += t[(y1 - 1) * width + (x1 - 1)];
                return (float)res;
            };

            float m = sum_at(intSum) / area;
            float s = std::sqrt(std::max(0.0f, (sum_at(intSqSum) / area) - (m * m)));
            float A = (s - w_min_stddev) / range;
            float threshold = m - (m * m - s) / ((global_mean + s) * (A + s) + 0.0001f);

            output_image.pixels[row_ptr + i] = (pixels[row_ptr + i] < threshold) ? 0 : 255;
        }
    }

    return output_image;
}

} // namespace imgclean::processors
*/
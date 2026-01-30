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

    const int width         = image.width;
    const int height        = image.height;
    const size_t num_pixels = image.pixels.size();
    const unsigned char* pixels = image.pixels.data();

    // 1. Pre-calculate Integral Images (O(N))
    // Matching the exact size of your input to maintain indexing parity
    std::vector<double> intSum(num_pixels, 0.0);
    std::vector<double> intSqSum(num_pixels, 0.0);

    for (int j = 0; j < height; ++j) {
        double rowSum = 0;
        double rowSqSum = 0;
        for (int i = 0; i < width; ++i) {
            const size_t idx = j * width + i;
            double val = static_cast<double>(pixels[idx]);
            rowSum += val;
            rowSqSum += val * val;

            if (j == 0) {
                intSum[idx] = rowSum;
                intSqSum[idx] = rowSqSum;
            } else {
                intSum[idx] = rowSum + intSum[(j - 1) * width + i];
                intSqSum[idx] = rowSqSum + intSqSum[(j - 1) * width + i];
            }
        }
    }

    const float global_mean = static_cast<float>(intSum[num_pixels - 1] / num_pixels);

    // 2. Prepare result buffers
    std::vector<float> windows_mean(num_pixels);
    std::vector<float> windows_stddev(num_pixels);
    float w_min_stddev = std::numeric_limits<float>::max();
    float w_max_stddev = std::numeric_limits<float>::lowest();

    // First Pass: Calculate local stats and global min/max stddev
    #pragma omp parallel for reduction(min:w_min_stddev) reduction(max:w_max_stddev) schedule(static)
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            const int x1 = std::max(0, i - half_window);
            const int y1 = std::max(0, j - half_window);
            const int x2 = std::min(width - 1, i + half_window);
            const int y2 = std::min(height - 1, j + half_window);
            const float area = static_cast<float>((x2 - x1 + 1) * (y2 - y1 + 1));

            // Logic to handle window sums without the +1 padding (safer for your GSImage)
            auto get_area_sum = [&](const std::vector<double>& table) {
                double res = table[y2 * width + x2];
                if (x1 > 0) res -= table[y2 * width + (x1 - 1)];
                if (y1 > 0) res -= table[(y1 - 1) * width + x2];
                if (x1 > 0 && y1 > 0) res += table[(y1 - 1) * width + (x1 - 1)];
                return static_cast<float>(res);
            };

            const float m = get_area_sum(intSum) / area;
            const float s = std::sqrt(std::max(0.0f, (get_area_sum(intSqSum) / area) - (m * m)));

            const size_t index = (size_t)j * width + i;
            windows_mean[index] = m;
            windows_stddev[index] = s;

            if (s < w_min_stddev) w_min_stddev = s;
            if (s > w_max_stddev) w_max_stddev = s;
        }
    }

    // 3. Final Pass: Binarization
    GSImage output_image;
    output_image.width     = width;
    output_image.height    = height;
    output_image.maxval    = image.maxval;
    output_image.exif_data = image.exif_data;
    output_image.pixels.resize(num_pixels);

    const float stddev_range = w_max_stddev - w_min_stddev;

    #pragma omp parallel for schedule(static)
    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            const size_t index = (size_t)j * width + i;
            const float s = windows_stddev[index];
            const float m = windows_mean[index];

            float adaptive_stddev = (stddev_range > 0.0001f) ? (s - w_min_stddev) / stddev_range : 0.0f;

            // Using your exact threshold logic from the original code provided
            float threshold = m - (m * m - s) / ((global_mean + s) * (adaptive_stddev + s) + 0.0001f);

            output_image.pixels[index] = (pixels[index] < threshold) ? 0 : 255;
        }
    }

    return output_image;
}

} // namespace imgclean::processors
*/
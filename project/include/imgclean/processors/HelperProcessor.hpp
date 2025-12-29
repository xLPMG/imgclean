#ifndef IMGCLEAN_PROCESSORS_HELPERPROCESSOR_HPP
#define IMGCLEAN_PROCESSORS_HELPERPROCESSOR_HPP

#include <imgclean/Constants.hpp>
#include <imgclean/GSImage.hpp>
#include <imgclean/PPMImage.hpp>

namespace imgclean
{
namespace processors
{
class HelperProcessor
{
public:
	//! Converts an RGB PPMImage to a normalized grayscale GSImage using linear approximation
	static GSImage rgb_to_linear_grayscale(const PPMImage& image)
	{
		const size_t total_pixels = static_cast<size_t>(image.width) * static_cast<size_t>(image.height);

		GSImage gray_image;
		gray_image.width     = image.width;
		gray_image.height    = image.height;
		gray_image.exif_data = image.exif_data; // preserve EXIF data
		gray_image.pixels.resize(total_pixels);
		std::vector<uint16_t> temp_pixels(total_pixels);

		uint16_t max_gray = 0;

#pragma omp parallel for simd schedule(static) reduction(max : max_gray) if (total_pixels >= MIN_SIZE)
		for (size_t i = 0; i < total_pixels; ++i)
		{
			const uint16_t r = image.pixels[i * 3 + 0];
			const uint16_t g = image.pixels[i * 3 + 1];
			const uint16_t b = image.pixels[i * 3 + 2];
			const float gray = R_scale * r + G_scale * g + B_scale * b;
			// +0.5f for rounding
			const uint16_t gray_val = static_cast<uint16_t>(gray + 0.5f);
			temp_pixels[i]          = gray_val;
			if (gray_val > max_gray) max_gray = gray_val;
		}

		// rescale to 0-255
		if (max_gray == 0) max_gray = 1; // avoid division by zero
		const float scale = 255.0f / max_gray;

#pragma omp parallel for schedule(static) if (total_pixels >= MIN_SIZE)
		for (size_t i = 0; i < total_pixels; ++i)
		{
			gray_image.pixels[i] = static_cast<uint8_t>(temp_pixels[i] * scale + 0.5f);
		}

		gray_image.maxval = 255;

		return gray_image;
	}

	//! Converts a grayscale GSImage to an RGB PPMImage
	static PPMImage grayscale_to_rgb(const GSImage& gray_image)
	{
		PPMImage rgb_image;
		rgb_image.width     = gray_image.width;
		rgb_image.height    = gray_image.height;
		rgb_image.maxval    = gray_image.maxval;
		rgb_image.exif_data = gray_image.exif_data; // preserve EXIF data
		rgb_image.pixels.resize(gray_image.width * gray_image.height * 3);

		const size_t total_pixels = static_cast<size_t>(gray_image.width) *
		                            static_cast<size_t>(gray_image.height);

#pragma omp parallel for schedule(static) if (total_pixels >= MIN_SIZE)
		for (size_t i = 0; i < total_pixels; ++i)
		{
			const uint16_t gray         = gray_image.pixels[i];
			rgb_image.pixels[i * 3 + 0] = gray;
			rgb_image.pixels[i * 3 + 1] = gray;
			rgb_image.pixels[i * 3 + 2] = gray;
		}

		return rgb_image;
	}
};
} // namespace processors
} // namespace imgclean
#endif // IMG_CLEAN_PROCESSORS_HELPERPROCESSOR_HPP

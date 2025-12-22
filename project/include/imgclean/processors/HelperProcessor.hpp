#ifndef IMG_CLEAN_PROCESSORS_HELPERPROCESSOR_HPP
#define IMG_CLEAN_PROCESSORS_HELPERPROCESSOR_HPP

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
		GSImage gray_image;
		gray_image.width  = image.width;
		gray_image.height = image.height;
		gray_image.pixels.resize(image.width * image.height);

		uint16_t max_gray = 0;
		for (int i = 0; i < image.width * image.height; ++i)
		{
			const uint16_t r = image.pixels[i * 3 + 0];
			const uint16_t g = image.pixels[i * 3 + 1];
			const uint16_t b = image.pixels[i * 3 + 2];
			const float gray       = 0.299f * r + 0.587f * g + 0.114f * b;
			// +0.5f for rounding
			const uint16_t gray_val    = static_cast<uint16_t>(gray + 0.5f);
			gray_image.pixels[i] = gray_val;
			if (gray_val > max_gray) max_gray = gray_val;
		}

		// rescale to 0-255
		if (max_gray == 0) max_gray = 1; // avoid division by zero
		const float scale = 255.0f / max_gray;
		for (int i = 0; i < image.width * image.height; ++i)
		{
			gray_image.pixels[i] = static_cast<uint8_t>(gray_image.pixels[i] * scale + 0.5f);
		}
		gray_image.maxval = 255;

		return gray_image;
	}

	//! Converts a grayscale GSImage to an RGB PPMImage
	static PPMImage grayscale_to_rgb(const GSImage& gray_image)
	{
		PPMImage rgb_image;
		rgb_image.width  = gray_image.width;
		rgb_image.height = gray_image.height;
		rgb_image.maxval = gray_image.maxval;
		rgb_image.pixels.resize(gray_image.width * gray_image.height * 3);

		for (int i = 0; i < gray_image.width * gray_image.height; ++i)
		{
			const uint16_t gray               = gray_image.pixels[i];
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

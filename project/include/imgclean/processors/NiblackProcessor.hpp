#ifndef IMGCLEAN_NIBLACKPROCESSOR_HPP
#define IMGCLEAN_NIBLACKPROCESSOR_HPP

#include <imgclean/GSImage.hpp>

namespace imgclean::processors
{
class NiblackProcessor
{
public:
	//! Applies the Niblack thresholding algorithm to a grayscale image.
	static GSImage apply(const GSImage& image);

private:
	//! Threshold factor
	static constexpr int window_size = 15;
	//! Half of the window size
	static constexpr int half_window = window_size / 2;
};
} // namespace imgclean::processors

#endif //IMGCLEAN_NIBLACKPROCESSOR_HPP

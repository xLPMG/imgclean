#ifndef IMGCLEAN_NICKPROCESSOR_HPP
#define IMGCLEAN_NICKPROCESSOR_HPP

#include <imgclean/GSImage.hpp>

namespace imgclean::processors
{
class NickProcessor
{
public:
	//! Applies the Nick thresholding algorithm: 
	//! T = m + k * sqrt{(sum(P_i^2 - m^2)) / (N*P)} 
	//! to a grayscale image.
	static GSImage apply(const GSImage& image);

private:
	//! Threshold factor
	static constexpr int window_size = 15;
	//! Half of the window size
	static constexpr int half_window = window_size / 2;
};
} // namespace imgclean::processors

#endif //IMGCLEAN_NICKPROCESSOR_HPP

#ifndef IMGCLEAN_SAUVOLAPROCESSOR_HPP
#define IMGCLEAN_SAUVOLAPROCESSOR_HPP

#include <imgclean/GSImage.hpp>

namespace imgclean::processors
{
class SauvolaProcessor
{
public:
	static GSImage apply(const GSImage& image);

private:
	//! Threshold factor
	static constexpr int window_size = 15;
	//! Half of the window size
	static constexpr int half_window = window_size / 2;
};
} // namespace imclean::processors

#endif //IMGCLEAN_SAUVOLAPROCESSOR_HPP

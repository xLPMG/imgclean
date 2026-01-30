#ifndef IMGCLEAN_PROCESSORS_BRADLEY_ROTH_PROCESSOR_HPP
#define IMGCLEAN_PROCESSORS_BRADLEY_ROTH_PROCESSOR_HPP

#include <imgclean/GSImage.hpp>

namespace imgclean
{
namespace processors
{
class BradleyRothProcessor
{
public:
	//! Applies the Bradley-Roth algorithm to a grayscale image.
	static GSImage apply(const GSImage& image);

private:
	//! Window size for local mean calculation
	static constexpr int window_size = 15;
	//! Half of the window size
	static constexpr int half_window = window_size / 2;
	//! Threshold factor
	static constexpr float t = 0.85f;
};
} // namespace processors
} // namespace imgclean
#endif // IMGCLEAN_PROCESSORS_BRADLEY_ROTH_PROCESSOR_HPP

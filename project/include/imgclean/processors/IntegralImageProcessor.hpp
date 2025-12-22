#ifndef IMG_CLEAN_PROCESSORS_INTEGRALIMAGEPROCESSOR_HPP
#define IMG_CLEAN_PROCESSORS_INTEGRALIMAGEPROCESSOR_HPP

#include <imgclean/GSImage.hpp>

namespace imgclean
{
namespace processors
{
class IntegralImageProcessor
{
public:
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
#endif // IMG_CLEAN_PROCESSORS_INTEGRALIMAGEPROCESSOR_HPP

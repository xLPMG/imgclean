#ifndef IMGCLEAN_PROCESSORS_BATAINEH_PROCESSOR_HPP
#define IMGCLEAN_PROCESSORS_BATAINEH_PROCESSOR_HPP

#include <imgclean/GSImage.hpp>

namespace imgclean::processors
{
class BatainehProcessor
{
public:
 	//! Applies the Bataineh et al. algorithm to a grayscale image.
	static GSImage apply(const GSImage& image);

private:
	//! Threshold factor
	static constexpr int window_size = 15;
	//! Half of the window size
	static constexpr int half_window = window_size / 2;
};
} // namespace imgclean::processors


#endif // IMGCLEAN_PROCESSORS_BATAINEH_PROCESSOR_HPP

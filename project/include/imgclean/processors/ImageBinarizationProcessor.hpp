#ifndef IMG_CLEAN_PROCESSORS_IMAGEBINARIZATION_HPP
#define IMG_CLEAN_PROCESSORS_IMAGEBINARIZATION_HPP

#include <imgclean/GSImage.hpp>

namespace imgclean::processors
{
class ImageBinarizationProcessor
{
public:
	//! Preprocesses the PPM Image with a binarization & local thresholding method
	static GSImage apply(const GSImage& image);

private:
	//! Threshold factor
	static constexpr int window_size = 15;
	//! Half of the window size
	static constexpr int half_window = window_size / 2;
};
} // namespace imgclean::processors


#endif //IMG_CLEAN_PROCESSORS_IMAGEBINARIZATION_HPP

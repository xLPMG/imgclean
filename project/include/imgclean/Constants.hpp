#ifndef IMGCLEAN_CONSTANTS_HPP
#define IMGCLEAN_CONSTANTS_HPP

#include <cstddef>

namespace imgclean
{
//! Minimum image size for OpenMP parallelization
constexpr size_t MIN_SIZE = 10000;

//! RGB to Grayscale conversion coefficients
constexpr float R_scale = 0.299f;
constexpr float G_scale = 0.587f;
constexpr float B_scale = 0.114f;

} // namespace imgclean

#endif // IMGCLEAN_CONSTANTS_HPP

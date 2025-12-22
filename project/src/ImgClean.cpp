#include "imgclean/ImgClean.hpp"

#include "imgclean/FileHandler.hpp"
#include "imgclean/ImageFormat.hpp"
#include "imgclean/PPMImage.hpp"
#include <imgclean/processors/HelperProcessor.hpp>
#include <imgclean/processors/IntegralImageProcessor.hpp>
#include <iostream>
#include <string>

namespace imgclean
{

bool ImgClean::check_format_support(const imgclean::ImageFormat& format, const std::string& path)
{
	if (format == imgclean::ImageFormat::PNG)
	{
#ifndef cimg_use_png
		std::cerr << "Error: PNG format not supported (libpng not found during build)\n";
		std::cerr << "File: " << path << "\n";
		return false;
#endif
	}
	else if (format == imgclean::ImageFormat::JPG)
	{
#ifndef cimg_use_jpeg
		std::cerr << "Error: JPEG format not supported (libjpeg not found during build)\n";
		std::cerr << "File: " << path << "\n";
		return false;
#endif
	}

	// Check if CImg is available for PNG/JPG
	if (format == imgclean::ImageFormat::PNG || format == imgclean::ImageFormat::JPG)
	{
#ifndef CIMG_FOUND
		std::cerr << "Error: CImg library not found during build - only PPM format is supported\n";
		std::cerr << "File: " << path << "\n";
		return false;
#endif
	}

	return true;
}

bool ImgClean::clean_image(const std::string& input_path, const std::string& output_path)
{
	/////////////////////////////////////////////////////////////////////////
	///// LOAD INPUT IMAGE
	/////////////////////////////////////////////////////////////////////////

	std::ios::sync_with_stdio(false);

	imgclean::FilePath input_file = imgclean::FileHandler::make_file_path(input_path);
	if (!check_format_support(input_file.format, input_path)) return false;

	imgclean::PPMImage image;
	if (!imgclean::FileHandler::load_image(input_file, image))
	{
		std::cerr << "Error: Failed to load image from '" << input_path << "'\n";
		std::cerr << "Hint: Ensure the file exists and has a valid file extension (.ppm, .png, .jpg, .jpeg)\n";
		return false;
	}

	/////////////////////////////////////////////////////////////////////////
	///// IMAGE PROCESSING
	/////////////////////////////////////////////////////////////////////////

	// Convert to grayscale
	imgclean::GSImage gray_image = imgclean::processors::HelperProcessor::rgb_to_linear_grayscale(image);
	// Apply integral image processor
	gray_image = imgclean::processors::IntegralImageProcessor::apply(gray_image);
	// Convert back to RGB
	image = imgclean::processors::HelperProcessor::grayscale_to_rgb(gray_image);

	/////////////////////////////////////////////////////////////////////////
	///// SAVE OUTPUT IMAGE
	/////////////////////////////////////////////////////////////////////////

	imgclean::FilePath output_file = imgclean::FileHandler::make_file_path(output_path);
	if (!imgclean::FileHandler::save_image(output_file, image))
	{
		std::cerr << "Error: Failed to save image to '" << output_path << "'\n";
		return false;
	}

	std::cout << "Saved image to '" << output_path << "'\n";
	return true;
}

} // namespace imgclean

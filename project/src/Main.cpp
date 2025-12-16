#include "imgclean/FileHandler.hpp"
#include "imgclean/ImageFormat.hpp"
#include "imgclean/PPMImage.hpp"
#include <cstdlib>
#include <iostream>
#include <string>

#ifdef MEASURE_PERFORMANCE
#include <chrono>
#endif

//! Check if the required format support is available based on compile-time definitions
bool check_format_support(const imgclean::ImageFormat& format, const std::string& path)
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

//! Print usage information
void print_usage(const char* program_name)
{
	std::cerr << "Usage: " << program_name << " -i <input> -o <output>\n";
	std::cerr << "Options:\n";
	std::cerr << "  -i, --input <file>   Input image file\n";
	std::cerr << "  -o, --output <file>  Output image file\n";
}

int main(int argc, char** argv)
{
/////////////////////////////////////////////////////////////////////////
///// INPUT READING & VALIDATION
/////////////////////////////////////////////////////////////////////////
	std::string input_path;
	std::string output_path;

	// Parse command line arguments
	for (int i = 1; i < argc; ++i)
	{
		std::string arg = argv[i];

		if (arg == "-i" || arg == "--input")
		{
			if (i + 1 < argc)
			{
				input_path = argv[++i];
			}
			else
			{
				std::cerr << "Error: --input requires a file path\n";
				print_usage(argv[0]);
				return EXIT_FAILURE;
			}
		}
		else if (arg == "-o" || arg == "--output")
		{
			if (i + 1 < argc)
			{
				output_path = argv[++i];
			}
			else
			{
				std::cerr << "Error: --output requires a file path\n";
				print_usage(argv[0]);
				return EXIT_FAILURE;
			}
		}
		else
		{
			std::cerr << "Error: Unknown option '" << arg << "'\n";
			print_usage(argv[0]);
			return EXIT_FAILURE;
		}
	}

	if (input_path.empty() || output_path.empty())
	{
		std::cerr << "Error: Both --input and --output are required\n";
		print_usage(argv[0]);
		return EXIT_FAILURE;
	}

/////////////////////////////////////////////////////////////////////////
///// START TIMER
/////////////////////////////////////////////////////////////////////////
#ifdef MEASURE_PERFORMANCE
	auto start_time = std::chrono::high_resolution_clock::now();
#endif

/////////////////////////////////////////////////////////////////////////
///// LOAD INPUT IMAGE
/////////////////////////////////////////////////////////////////////////

	std::ios::sync_with_stdio(false);

	imgclean::FilePath input_file = imgclean::FileHandler::make_file_path(input_path);
	if (!check_format_support(input_file.format, input_path)) return EXIT_FAILURE;

	imgclean::PPMImage image;
	if (!imgclean::FileHandler::load_image(input_file, image))
	{
		std::cerr << "Error: Failed to load image from '" << input_path << "'\n";
		return EXIT_FAILURE;
	}

/////////////////////////////////////////////////////////////////////////
///// IMAGE PROCESSING
/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
///// SAVE OUTPUT IMAGE
/////////////////////////////////////////////////////////////////////////

	imgclean::FilePath output_file = imgclean::FileHandler::make_file_path(output_path);
	if (!imgclean::FileHandler::save_image(output_file, image))
	{
		std::cerr << "Error: Failed to save image to '" << output_path << "'\n";
		return EXIT_FAILURE;
	}

	std::cout << "Saved image to '" << output_path << "'\n";

#ifdef MEASURE_PERFORMANCE
	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
	std::cout << "Processing time: " << duration << " ms\n";
#endif

	return EXIT_SUCCESS;
}

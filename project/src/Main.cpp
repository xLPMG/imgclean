#include "imgclean/ImgClean.hpp"
#include <cstdlib>
#include <iostream>
#include <string>

#ifdef MEASURE_PERFORMANCE
# include <chrono>
#endif

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
///// IMAGE PROCESSING
/////////////////////////////////////////////////////////////////////////
#ifdef MEASURE_PERFORMANCE
	auto start_time = std::chrono::high_resolution_clock::now();
#endif

	// TODO: as io parameter
	// integral or adaptive
	std::string approach = "adaptive";
	bool success = imgclean::ImgClean::clean_image(input_path, output_path, approach);
	if (!success)
	{
		std::cerr << "Error: Image cleaning failed\n";
		return EXIT_FAILURE;
	}

#ifdef MEASURE_PERFORMANCE
	auto end_time = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
	std::cout << "Processing time: " << duration << " ms\n";
#endif

	return EXIT_SUCCESS;
}

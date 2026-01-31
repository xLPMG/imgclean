# imgclean - project

This directory contains the **C++ source code** for the imgclean tool. 

## Building

The build system we use is CMake version 4.0 or higher. Aside from CMake, a few other dependencies are required. As a minimum, you will need a C++20 and OpenMP capable compiler, as well as the OpenMP library itself. 

If you wish to use the PNG image format support, you will also need the libpng library installed on your system. Should you want to use JPEG images as well, the libjpeg library is also required. For both PNG and JPEG support, the CImg library needs to be available. Note that libjpeg, libpng and CImg are only necessary if you want to use those specific image formats; imgclean can simply use the PPM format without any additional dependencies.

After ensuring that all dependencies are installed, building the project is straightforward. From the `project` directory, create a build directory and navigate into it:

```bash
mkdir build
cd build
```

Then, run CMake to configure the build system:

```bash
cmake ..
```

Here, you can specify options such as a specific compiler, a build type (e.g., Release, Debug or Testing), or enable time measurement:

```bash
cmake -DCMAKE_CXX_COMPILER=g++-15 -DCMAKE_BUILD_TYPE=Release -DMEASURE_PERFORMANCE=ON ..
```

Finally, compile the project using:

```bash
cmake --build . --parallel 11
```

You may replace `11` with the number of CPU cores you wish to utilize for the build process.

## Running Tests

Should you wish to run the tests, make sure to configure the build with the `Testing` build type first, build the project, and then execute the tests using CTest:

```bash
cmake -DCMAKE_BUILD_TYPE=Testing ..
cmake --build . --parallel 11
ctest --output-on-failure --parallel 11
```

## Usage

### Command Line Interface

After building the project, the `imgclean` executable will be located in the build directory. You can run it from the command line as follows:

```bash
./imgclean -i <input> -o <output> [-a <approach>]
```

Running it without any arguments will display a help message detailing the available options and usage instructions:

```
Error: Both --input and --output are required
Usage: ./build/imgclean -i <input> -o <output> [-a <approach>]
Options:
  -i, --input <file>      Input image file
  -o, --output <file>     Output image file
  -a, --approach <type>   Image processing approach (default: bradley-roth)
                          Supported options: nick, sauvola, niblack, bataineh, bradley-roth
```

### C++ Library

When building the project, a static library named `libimgclean.a` is also created in the build directory. You can link this library to your own C++ projects to utilize the image cleaning functionalities programmatically. The method `imgclean::ImgClean::clean_image` will be your entry point for the image processing task.
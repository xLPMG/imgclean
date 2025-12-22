cmake_minimum_required(VERSION 4.0)
project(imgclean LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

###############################################################################
## build options ###############################################################
###############################################################################

option(MEASURE_PERFORMANCE "Enable performance timing in the binary" OFF)

set(IS_TESTING_BUILD OFF)
if (CMAKE_BUILD_TYPE STREQUAL "Testing")
    set(IS_TESTING_BUILD ON)
endif()

###############################################################################
## file discovery ##############################################################
###############################################################################

file(GLOB_RECURSE sources src/*.cpp include/imgclean/*.h)
file(GLOB_RECURSE tests   test/*.cpp)
file(GLOB_RECURSE data    res/*)

# Exclude main files from library sources
list(FILTER sources EXCLUDE REGEX ".*[Mm]ain.*\\.cpp$")
list(FILTER tests   EXCLUDE REGEX ".*[Mm]ain.*\\.cpp$")

# Exclude test resources in non-testing builds
if (NOT IS_TESTING_BUILD)
    list(FILTER data EXCLUDE REGEX ".*/res/test/.*")
endif()

message("--------------------------------------------------")
message(STATUS "Sources: ${sources}")
message(STATUS "Tests:   ${tests}")
message(STATUS "Data:    ${data}")
message("--------------------------------------------------")

###############################################################################
## targets ####################################################################
###############################################################################

# Core static library
add_library(imgclean_lib STATIC ${sources})
target_include_directories(imgclean_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Standalone CLI executable
add_executable(imgclean src/Main.cpp)
target_link_libraries(imgclean PRIVATE imgclean_lib)

# Tests
if (IS_TESTING_BUILD)
    add_executable(imgclean_tests test/Main.cpp ${tests})
    target_link_libraries(imgclean_tests PRIVATE imgclean_lib)
endif()

# Compile-time flags shared by CLI and tests
if (MEASURE_PERFORMANCE)
    target_compile_definitions(imgclean_lib PUBLIC MEASURE_PERFORMANCE)
endif()

if (IS_TESTING_BUILD)
    enable_testing()
    add_test(NAME imgclean_tests COMMAND imgclean_tests)
endif()

# Copy resource files near executable
file(COPY ${data} DESTINATION resources)

###############################################################################
## dependencies ###############################################################
###############################################################################

find_package(OpenMP REQUIRED)
target_link_libraries(imgclean_lib PUBLIC OpenMP::OpenMP_CXX)

if (IS_TESTING_BUILD)
    add_subdirectory(include/catch2)
    target_link_libraries(imgclean_tests PRIVATE catch2)
endif()

###############################################################################
## CImg + optional image libraries ############################################
###############################################################################

# CImg header-only
find_path(CIMG_INCLUDE_DIR CImg.h
    PATHS /usr/local/include /opt/homebrew/include /usr/include
)

if (CIMG_INCLUDE_DIR)
    message(STATUS "Found CImg: ${CIMG_INCLUDE_DIR}")
    target_compile_definitions(imgclean_lib PUBLIC CIMG_FOUND)
    target_include_directories(imgclean_lib PUBLIC ${CIMG_INCLUDE_DIR})
else()
    message(WARNING "CImg not found - only PPM_ASCII format will be supported")
endif()

# Optional PNG + JPEG
find_package(PNG QUIET)
find_package(JPEG QUIET)

if (PNG_FOUND)
    message(STATUS "Found PNG: ${PNG_LIBRARIES}")
    target_compile_definitions(imgclean_lib PUBLIC cimg_use_png)
    target_link_libraries(imgclean_lib PUBLIC PNG::PNG)
else()
    message(STATUS "PNG not found - PNG support disabled")
endif()

if (JPEG_FOUND)
    message(STATUS "Found JPEG: ${JPEG_LIBRARIES}")
    target_compile_definitions(imgclean_lib PUBLIC cimg_use_jpeg)
    target_link_libraries(imgclean_lib PUBLIC JPEG::JPEG)
else()
    message(STATUS "JPEG not found - JPEG support disabled")
endif()

###############################################################################
## packaging ##################################################################
###############################################################################

# # all install commands get the same destination. this allows us to use paths
# # relative to the executable.
# install(TARGETS imgclean DESTINATION imgclean_destination)
# # this is basically a repeat of the file copy instruction that copies the
# # resources in the build directory, but here we tell CMake that we want it
# # in the package
# install(DIRECTORY resources DESTINATION imgclean_destination)
# # now comes everything we need, to create a package
# # there are a lot more variables you can set, and some
# # you need to set for some package types, but we want to
# # be minimal here
# set(CPACK_PACKAGE_NAME "imgclean")
# set(CPACK_PACKAGE_VERSION "0.0.0")
# set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "TODO")

# # we don't want to split our program up into several things
# set(CPACK_MONOLITHIC_INSTALL 1)

# # This must be last
# include(CPack)

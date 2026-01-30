cmake_minimum_required(VERSION 4.0)
project(imgclean LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

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

# Exclude macOS finder metadata files
list(FILTER data EXCLUDE REGEX ".*/\\.DS_Store$")

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
# Set output name to avoid "_lib" suffix
set_target_properties(imgclean_lib PROPERTIES OUTPUT_NAME imgclean)
target_include_directories(imgclean_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Precompiled headers (PCH)
target_precompile_headers(imgclean_lib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/imgclean/pch.hpp)

# Standalone CLI executable
add_executable(imgclean src/Main.cpp)
target_link_libraries(imgclean PRIVATE imgclean_lib)
target_precompile_headers(imgclean REUSE_FROM imgclean_lib)

# Tests
if (IS_TESTING_BUILD)
    add_executable(imgclean_tests test/Main.cpp ${tests})
    target_link_libraries(imgclean_tests PRIVATE imgclean_lib)
    target_precompile_headers(imgclean_tests REUSE_FROM imgclean_lib)
endif()

if (IS_TESTING_BUILD)
    enable_testing()
    add_test(NAME imgclean_tests COMMAND imgclean_tests)
endif()

# Compiler flags
# We set the same flags for the standalone executable and tests as well, 
# to ensure that the PCH can be used across all targets.
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(imgclean_lib PRIVATE $<$<CONFIG:Release>:-O3 -march=native>)
    target_compile_options(imgclean PRIVATE $<$<CONFIG:Release>:-O3 -march=native>)
    if (IS_TESTING_BUILD)
        target_compile_options(imgclean_tests PRIVATE $<$<CONFIG:Release>:-O3 -march=native>)
    endif()
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(imgclean_lib PRIVATE $<$<CONFIG:Release>:/O2>)
    target_compile_options(imgclean PRIVATE $<$<CONFIG:Release>:/O2>)
    if (IS_TESTING_BUILD)
        target_compile_options(imgclean_tests PRIVATE $<$<CONFIG:Release>:/O2>)
    endif()
endif()

# Additional settings
if (MEASURE_PERFORMANCE)
    target_compile_definitions(imgclean_lib PUBLIC MEASURE_PERFORMANCE)
endif()

# Copy resource files near executable, preserving directory structure
# In testing builds, copy the full res/ tree so tests can access res/test/*
# In non-testing builds, copy the full res/ tree and remove res/test afterwards
if (IS_TESTING_BUILD)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
else()
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
    file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/res/test)
endif()

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

# Optional PNG + JPEG + EXIF support
find_package(PNG QUIET)
find_package(JPEG QUIET)
find_package(EXIF QUIET)

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

if (EXIF_FOUND)
    message(STATUS "Found EXIF: ${EXIF_LIBRARIES}")
    target_compile_definitions(imgclean_lib PUBLIC EXIF_FOUND)
    target_link_libraries(imgclean_lib PUBLIC ${EXIF_LIBRARIES})
else()
    message(STATUS "EXIF not found - EXIF support disabled")
endif()

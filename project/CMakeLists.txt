cmake_minimum_required(VERSION 4.0)
project(imgclean)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

###############################################################################
## build options ###############################################################
###############################################################################

option(MEASURE_PERFORMANCE "Enable performance timing in the binary" OFF)

# Treat a custom build type "Testing" specially
set(IS_TESTING_BUILD OFF)
if (CMAKE_BUILD_TYPE STREQUAL "Testing")
    set(IS_TESTING_BUILD ON)
endif()

###############################################################################
## file globbing ##############################################################
###############################################################################

file(GLOB_RECURSE sources   src/*.cpp include/imgclean/*.h)
file(GLOB_RECURSE tests     test/*.cpp)
file(GLOB_RECURSE data      res/*)

# Exclude main from sources
list(FILTER sources EXCLUDE REGEX ".*[Mm]ain.*\\.cpp$")
list(FILTER tests EXCLUDE REGEX ".*[Mm]ain.*\\.cpp$")

# In non-testing builds, exclude test resources from data
if (NOT IS_TESTING_BUILD)
    list(FILTER data EXCLUDE REGEX ".*/res/test/.*")
endif()

message("--------------------------------------------------")
message(STATUS "Sources: ${sources}")
message(STATUS "Tests: ${tests}")
message(STATUS "Data: ${data}")
message("--------------------------------------------------")

###############################################################################
## target definitions #########################################################
###############################################################################

add_executable(imgclean         src/Main.cpp    ${sources} ${data})
if (IS_TESTING_BUILD)
    add_executable(imgclean_tests   test/Main.cpp   ${tests} ${sources} ${data})
endif()
target_include_directories(imgclean PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
if (IS_TESTING_BUILD)
    target_include_directories(imgclean_tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()

if (MEASURE_PERFORMANCE)
    message(STATUS "MEASURE_PERFORMANCE: enabled")
    target_compile_definitions(imgclean PUBLIC MEASURE_PERFORMANCE)
endif()

if (IS_TESTING_BUILD)
    enable_testing()
    add_test(NAME imgclean_tests COMMAND imgclean_tests)
endif()

# this copies all resource files in the build directory
# we need this, because we want to work with paths relative to the executable
file(COPY ${data} DESTINATION resources)

###############################################################################
## dependencies ###############################################################
###############################################################################

find_package(OpenMP REQUIRED)
target_link_libraries(imgclean PUBLIC OpenMP::OpenMP_CXX)

if (IS_TESTING_BUILD)
    add_subdirectory(include/catch2)
    target_link_libraries(imgclean_tests PRIVATE catch2 PRIVATE OpenMP::OpenMP_CXX)
endif()

###############################################################################
## CImg (header-only) and optional image libs #################################
###############################################################################

find_path(CIMG_INCLUDE_DIR CImg.h
    PATHS /usr/local/include /opt/homebrew/include /usr/include
)
if (CIMG_INCLUDE_DIR)
    message(STATUS "Found CImg: ${CIMG_INCLUDE_DIR}")
    target_compile_definitions(imgclean PUBLIC CIMG_FOUND)
    if (IS_TESTING_BUILD)
        target_compile_definitions(imgclean_tests PUBLIC CIMG_FOUND)
    endif()
    target_include_directories(imgclean PUBLIC ${CIMG_INCLUDE_DIR})
    if (IS_TESTING_BUILD)
        target_include_directories(imgclean_tests PUBLIC ${CIMG_INCLUDE_DIR})
    endif()
else()
    message(WARNING "CImg not found - only PPM_ASCII format will be supported")
endif()

find_package(PNG QUIET)
find_package(JPEG QUIET)
if (PNG_FOUND)
    message(STATUS "Found PNG: ${PNG_LIBRARIES}")
    target_compile_definitions(imgclean PUBLIC cimg_use_png)
    if (IS_TESTING_BUILD)
        target_compile_definitions(imgclean_tests PUBLIC cimg_use_png)
    endif()
    target_link_libraries(imgclean PUBLIC PNG::PNG)
    if (IS_TESTING_BUILD)
        target_link_libraries(imgclean_tests PUBLIC PNG::PNG)
    endif()
else()
    message(STATUS "PNG not found - PNG support disabled")
endif()
if (JPEG_FOUND)
    message(STATUS "Found JPEG: ${JPEG_LIBRARIES}")
    target_compile_definitions(imgclean PUBLIC cimg_use_jpeg)
    if (IS_TESTING_BUILD)
        target_compile_definitions(imgclean_tests PUBLIC cimg_use_jpeg)
    endif()
    target_link_libraries(imgclean PUBLIC JPEG::JPEG)
    if (IS_TESTING_BUILD)
        target_link_libraries(imgclean_tests PUBLIC JPEG::JPEG)
    endif()
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

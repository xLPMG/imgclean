cmake_minimum_required(VERSION 4.0)
project(imgclean)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

###############################################################################
## file globbing ##############################################################
###############################################################################

file(GLOB_RECURSE sources   src/*.cpp include/imgclean/*.h)
file(GLOB_RECURSE tests     test/*.cpp)
file(GLOB_RECURSE data      res/*)

# Exclude main from sources
list(FILTER sources EXCLUDE REGEX ".*main.*\\.cpp$")
list(FILTER tests EXCLUDE REGEX ".*main.*\\.cpp$")

message("--------------------------------------------------")
message(STATUS "Sources: ${sources}")
message(STATUS "Tests: ${tests}")
message(STATUS "Data: ${data}")
message("--------------------------------------------------")

###############################################################################
## target definitions #########################################################
###############################################################################

add_executable(imgclean         src/main.cpp    ${sources} ${data})
add_executable(imgclean_tests   test/main.cpp   ${tests} ${sources} ${data})

enable_testing()
add_test(NAME imgclean_tests COMMAND imgclean_tests)

# just for example add some compiler flags
# target_compile_options(imgclean PUBLIC -std=c++1y -Wall -Wfloat-conversion)
# this lets me include files relative to the root source directory with a <> pair
# target_include_directories(imgclean PUBLIC src/main include)

# this copies all resource files in the build directory
# we need this, because we want to work with paths relative to the executable
file(COPY ${data} DESTINATION resources)

###############################################################################
## dependencies ###############################################################
###############################################################################

find_package(OpenMP REQUIRED)
target_link_libraries(imgclean PUBLIC OpenMP::OpenMP_CXX)

add_subdirectory(include/catch2)
target_link_libraries(imgclean_tests PRIVATE catch2 PRIVATE OpenMP::OpenMP_CXX)

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

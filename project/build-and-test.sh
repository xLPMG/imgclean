mkdir -p build
cd build
cmake -DCMAKE_CXX_COMPILER=g++-15 -DCMAKE_BUILD_TYPE=Release -DMEASURE_PERFORMANCE=ON ..
cmake --build . --parallel 11
ctest --output-on-failure --parallel 11
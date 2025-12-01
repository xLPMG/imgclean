mkdir -p build
cd build
cmake -DCMAKE_CXX_COMPILER=g++-15 -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel 11
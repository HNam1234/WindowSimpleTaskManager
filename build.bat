mkdir build
cd build
conan install .. --output-folder=. --build=missing -s build_type=Debug

cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build .

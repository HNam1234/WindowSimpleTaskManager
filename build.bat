mkdir build
cd build
conan install .. --output-folder=. --build=missing
cmake .. 
cmake --build . --config Release
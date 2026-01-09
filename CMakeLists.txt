cmake_minimum_required(VERSION 3.15)
project(WindowSimpleTaskManager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find spdlog (installed by Conan)
find_package(spdlog REQUIRED)

# Add executable
add_executable(main main.cpp)

target_link_libraries(main PRIVATE spdlog::spdlog)
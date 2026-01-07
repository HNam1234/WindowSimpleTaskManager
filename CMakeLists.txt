cmake_minimum_required(VERSION 3.15)
project(WindowSimpleTaskManager)

# Include Conan toolchain
include(${CMAKE_BINARY_DIR}/conan_toolchain.cmake)

# Find spdlog package installed by Conan
find_package(spdlog REQUIRED)

# Add executable
add_executable(ReadWindowCpuUsage ReadWindowCpuUsage.cpp)

# Link spdlog
target_link_libraries(ReadWindowCpuUsage spdlog::spdlog)
cmake_minimum_required(VERSION 3.10)

project(Raytrace)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Ensure proper flags for profiling in Release with Debug Info
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-O2 -g -fno-omit-frame-pointer -fno-inline -fopt-info-vec-optimized=vec.log ")

# Optional: Make sure CMake doesn't overwrite these with toolchain defaults
set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)

# Source files
file(GLOB SOURCES "src/*.cpp")

# Info message
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Executable
add_executable(MyExecutable ${SOURCES})
target_include_directories(MyExecutable PUBLIC include)


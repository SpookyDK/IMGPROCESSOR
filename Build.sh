#!/bin/bash

set -e

# Clear terminal and create build directory
mkdir -p build
cd build

# Run CMake and build
cmake ..
make

# Go back to project root
cd ..

# Determine which executable to run
if [[ "$1" == "test" ]]; then
    echo -e "\n--- Running tests ---"
    perf stat ./build/MyTests
else
    echo -e "\n--- Running program ---"
    perf stat ./build/MyExecutable
fi

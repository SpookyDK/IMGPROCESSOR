#!/bin/bash

set -e

clear
mkdir -p build

cd build

cmake ..
bear -- make


mv compile_commands.json ../
echo -e "\n--- Running program ---"
cd ../
perf stat ./build/MyExecutable



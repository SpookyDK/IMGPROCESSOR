#!/bin/bash

set -e

clear
mkdir -p build

cd build

cmake ..
bear -- make

echo -e "\n--- Running program ---"
mv compile_commands.json ../
perf stat ./MyExecutable



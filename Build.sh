#!/bin/bash

set -e

mkdir -p build

cd build

cmake ..
bear -- make

echo -e "\n--- Running program ---"
mv compile_commands.json ../
./MyExecutable



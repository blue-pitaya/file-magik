#!/bin/sh

set -e

cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build

cp build/compile_commands.json .
./build/main

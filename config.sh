#!/bin/bash -e

mkdir BUILD
cd BUILD

export CXX=clang++
cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=Yes ..

cd ..
ln -sf BUILD/compile_commands.json

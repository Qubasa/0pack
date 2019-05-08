#!/usr/bin/env bash

mkdir -p build
cd build || exit
cmake ..
make
cd .. || exit

ln -s -f build/compile_commands.json .

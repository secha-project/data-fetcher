#!/bin/bash

set -e

rm -rf build
cmake -S . -B build
cd build
make clean
make
cd ..

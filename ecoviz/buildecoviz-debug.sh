#!/bin/sh

rm -r build
mkdir build
#rm CMakeCache.txt -- removed by dir delete
cd build
#cmake -DCMAKE_CXX_COMPILER=g++-7 ..
cmake -DCMAKE_BUILD_TYPE=Debug ..
cd build
make

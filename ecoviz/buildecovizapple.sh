#!/bin/sh

rm -r ./build
mkdir build
rm CMakeCache.txt
cd build
cmake -DQt6_DIR=$(brew --prefix qt6)/lib/cmake/Qt6 ..
cmake -DQt6Widgets_DIR=$/opt/homebrew/opt/qt@6/lib/cmake/Qt6Widgets ..
# cmake -DGLUT_ROOT=$/opt/homebrew/opt/freeglut ..
# cmake -DCMAKE_CXX_FLAGS="-std=c++17" ..
# cmake -DCMAKE_CXX_COMPILER=g++ ..
cmake -DCMAKE_BUILD_TYPE=Release ..
make

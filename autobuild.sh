#!/bin/bash
set -e
rm -rf build
mkdir build
cd build
cmake ..
make

set -x
rm -rf 'pwd'/build/*
cd 'pwd'/build &&
	cmake .. &&
	make


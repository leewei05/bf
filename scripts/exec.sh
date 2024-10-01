#!/bin/sh

# set -x
make -j -C build/
./build/bf $2 -c $1
clang -O3 -g ./bf.s -o build/a.out
./build/a.out

# clean up
rm ./bf.s

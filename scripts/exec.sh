#!/bin/sh

set -x
make -j -C build/
./build/bf -c $1
clang -O3 ./bf.s -o build/a.out
./build/a.out

# clean up
rm ./bf.s

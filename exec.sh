#!/bin/sh

set -x
make -C build/
./build/bf -c $1
gcc -O3 ./bf.s driver.c -o build/a.out
./build/a.out

# clean up
rm ./bf.s

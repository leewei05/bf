#!/bin/sh

set -x
./build/bf -c $1
gcc -O3 ./bf.s driver.c
./a.out

#!/bin/bash

rm -rf perf/*

for d in ./brainfuck-benchmark/benches/* ; do
    b=`basename $d`
    filename="${b%%.*}"
    echo "$filename baseline" > "./perf/$filename.txt"
    { time ./scripts/exec.sh "$d" 1> /dev/null; } 2>> "./perf/$filename.txt"
    echo >> "./perf/$filename.txt"
    echo "----------------------------------------" >> "./perf/$filename.txt"
    echo "$filename loop optimization" >> "./perf/$filename.txt"
    { time ./scripts/exec.sh "$d" -l 1> /dev/null; } 2>> "./perf/$filename.txt"
    echo  >> "./perf/$filename.txt"
    echo "----------------------------------------" >> "./perf/$filename.txt"
    echo "$filename vector optimization" >> "./perf/$filename.txt"
    { time ./scripts/exec.sh "$d" -v 1> /dev/null; } 2>> "./perf/$filename.txt"
    echo >> "./perf/$filename.txt"
    echo "----------------------------------------" >> "./perf/$filename.txt"
    echo "$filename both optimizations" >> "./perf/$filename.txt"
    { time ./scripts/exec.sh "$d" -O 1> /dev/null; } 2>> "./perf/$filename.txt"
done
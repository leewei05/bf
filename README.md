## BF Interpreter

### Quickstart

```
git clone git@github.com:leewei05/bf.git
cd bf
mkdir build
cd build
cmake ..
make -j
git submodule update --init
./bf ../brainfuck-benchmark/benches/hello.b
```

### Profile

Enable profiling option with `-p`.

```
./bf -p ../brainfuck-benchmark/benches/hello.b
```

The following is part of the profiling output of `bottles.b`.
Profiling output consist of three parts.
The first part is each instruction's count.

```
Instructions profile:
0: +: 1
1: +: 1
...
```

The second part is a list of simple loop in descending order. `Position: Loop info : Loop body count`

```
Simple loops:
463: [-]: 11371
1043: [-]: 11371
2231: [-]: 11371
1795: [-]: 5202
1799: [-]: 5202
29: [-]: 5145
...
```

The third part is a list of non-simple loop in descending order.

```
Non-simple loops:
95: [<+>-]: 23100
1861: [<+>-]: 22650
38: [>+>+<<-]: 4950
49: [<<+>>-]: 4950
1804: [>+>+<<-]: 4851
1815: [<<+>>-]: 4851
...
```

### Compile

We can compile bf into native x86-64 assembly code with option `-c`. With the use of `exec.sh`, we can compile source bf file into
assembly file and generate an executable.

```
./exec.sh brainfuck-benchmark/benches/hello.b
+ make -C build/
[100%] Built target bf
+ ./build/bf -c brainfuck-benchmark/benches/hello.b
Finished compiling!
Normal Termination!
+ clang -O3 ./bf.s -o build/a.out
+ ./build/a.out
Hello World!
+ rm ./bf.s
```

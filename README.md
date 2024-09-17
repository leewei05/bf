## bf interpreter

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

Enable profiling option with `-p`.

```
./bf -p ../brainfuck-benchmark/benches/hello.b
```

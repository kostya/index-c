# C Stress Test

An amalgamated, simplified version of the [LangArena](https://github.com/kostya/LangArena)
suite for the C language.

LangArena is a collection of 50 real-world-inspired tasks: JSON, Base64, CSV,
neural networks, compression, maze A*, graph algorithms, sorting, hashing,
interpreters, parallel matmul, and more.

The file (index.c) serves three purposes at once:

- **Compiler stress test** - a single ~500 KB translation unit exercises the
  front end, optimizer, and back end.
- **Runtime benchmark** - each test runs for roughly a second, so the total
  runtime is around 52 seconds on my hardware.
- **Correctness verifier** - every test checks its result against a checksum.
  If any test fails, the program exits with code 1.

Fully "all in": no external dependencies beyond the standard libraries.

All parameters have been tuned for GCC 16 -O2. Timings may differ on other
hardware.

### Build and run:

    gcc index.c -O3 -lm -lpthread -o ./index
    ./index

### Fast run, only test checksums

    TEST=1 ./index

### Source includes:

* https://github.com/DaveGamble/cJSON
  MIT License (Copyright (c) 2009-2017 Dave Gamble and cJSON contributors)

* https://github.com/troydhanson/uthash
  Copyright (c) 2005-2026, Troy D. Hanson
  https://troydhanson.github.io/uthash/

* https://github.com/kokke/tiny-regex-c
  All material in this repository is in the public domain.

* https://github.com/wareya/Remimu/
  Creative Commons Legal Code

### MIT License

# Experiment: GCC and Clang performance over 12 years.

I measured runtime, compilation time, compilation RSS and binary size for `index.c` across GCC 4–16 and Clang 3–23, using the flags `-O0`, `-O1`, `-O2`, `-O3`, `-Os`, `-Oz`, `-Ofast`, and `-O3 -march=native`. Experiment date: 2026-10-31.

Environment:

- **CPU:** Ryzen 3800X
- **RAM:** 78 GB DDR4-3200
- **OS:** Linux arch 7.2.7
- **Docker:** 29.8.1

The benchmarks were executed in Docker, with images pulled from Docker Hub. This can introduce uncertainty into the results, since each image may contain its own version of glibc and other unknown factors. GCC builds used the official GCC images; Clang builds used images from teeks99.

**GCC:** 4.9.4, 5.5.0, 6.5.0, 7.5.0, 8.5.0, 9.5.0, 10.5.0, 11.5.0, 12.5.0, 13.5.0, 14.4.0, 15.3.0, 16.2.0

**Clang:** 3.4, 3.6.0, 3.9.1, 4.0.0, 5.0.2, 6.0.1, 7.1.0, 8.0.1, 9.0.0, 10.0.1, 11.1.0, 12.0.1, 13.0.1, 14.0.6, 15.0.7, 16.0.6, 17.0.6, 18.1.8, 19.1.7, 20.1.8, 21.1.8, 22.1.8, 23.1.4

### Results for `-O1`, `-O2`, `-O3`

![plot](https://raw.githubusercontent.com/kostya/stuff/master/index-c/plot1_runtime.png)

![plot](https://raw.githubusercontent.com/kostya/stuff/master/index-c/plot1_compile.png)

Regarding the slowdown in GCC compile time (in `-O2`/`-O3` mode), it is clearly reproducible with two simple commands (using the official GCC images):

```
docker run --rm -v `pwd`:/src -w /src gcc:7 bash -c 'time gcc -O3 index.c -lm -lpthread -o /tmp/index'
```
real  0m2.663s

```
docker run --rm -v `pwd`:/src -w /src gcc:16 bash -c 'time gcc -O3 index.c -lm -lpthread -o /tmp/index'
```
real  0m4.661s

This is also confirmed by this source: https://github.com/lac-dcc/BenchGen/wiki/Comparing-gcc-versions#1 `Comparing version 5 and version 14, there is an increase of about 42% over a span of 9 years.`

It's also worth noting that `index.c` is a large (500 KB) amalgamated file. GCC may struggle with this kind of use case, which is actually not typical. This was not tested on smaller files - that is material for another experiment.

### Evolution plot for `-O2`

![plot](https://raw.githubusercontent.com/kostya/stuff/master/index-c/plot6.png)

### No optimizations: `-O0`

![plot](https://raw.githubusercontent.com/kostya/stuff/master/index-c/plot2_runtime.png)

![plot](https://raw.githubusercontent.com/kostya/stuff/master/index-c/plot2_compile.png)

### Compile memory, Mb

![plot](https://raw.githubusercontent.com/kostya/stuff/master/index-c/plot7_compile_mem.png)

### Binary size, Kb

![plot](https://raw.githubusercontent.com/kostya/stuff/master/index-c/plot3_binary_size.png)

### Results for `-Os`, `-Oz` vs `-O2`

![plot](https://raw.githubusercontent.com/kostya/stuff/master/index-c/plot4_runtime.png)

![plot](https://raw.githubusercontent.com/kostya/stuff/master/index-c/plot4_compile.png)

### Results for `-Ofast`, `-O3 -march=native` vs `-O3`

![plot](https://raw.githubusercontent.com/kostya/stuff/master/index-c/plot5_runtime.png)

![plot](https://raw.githubusercontent.com/kostya/stuff/master/index-c/plot5_compile.png)

The tested processor came out in 2019, but the effect of `-march=native` only started to matter in 2022.

* [reproduce script](https://github.com/kostya/index-c/blob/master/experiment/run.rb)
* [raw data](https://github.com/kostya/index-c/blob/master/experiment/merged.js)

# Experiment2: Compare C compilers on index.c

* [clang](https://clang.llvm.org): `clang version 23.1.5`
* [gcc](https://gcc.gnu.org): `gcc (GCC) 16.2.1 20260810`
* [tcc](https://bellard.org/tcc/): `tcc version 0.9.28rc 2026-01-17 makepkg@b39cbc70 (x86_64 Linux)`
* [cproc](https://github.com/michaelforney/cproc): `d1c53dd`
* [mycc](https://github.com/kostya/myc): `mycc 0.13.0-a00f054 c11-like compiler (backend: unknown) (https://github.com/kostya/myc)`
* [antcc](https://codeberg.org/lsof/antcc): `antcc version 0.4.7-4-gcd7c80c`
* [madc](https://github.com/derekbsnider/madc): `madc 0.106.1`
* [zig-cc](https://ziglang.org): `0.17.1`
* [mucc](https://github.com/Dom-Yan/Mucc): `mucc: error: version: No such file or directory`
* [cppgm](https://github.com/vishvananda/cppgm): `cppgm 0.11.1 (host configuration: g++)`

| Compiler | Backend | Flags | Compile (s) | Run (s) | Status |
|----------|---------|-------|-------------|---------|--------|
| gcc | - | -O0 | 0.7507 | 132.7446 | OK |
| gcc | - | -O1 | 1.8708 | 57.3448 | OK |
| gcc | - | -O2 | 3.6382 | 52.1492 | OK |
| gcc | - | -O3 | 4.7649 | 50.5582 | OK |
| clang | LLVM | -O0 | 0.3468 | 142.7883 | OK |
| clang | LLVM | -O1 | 1.6639 | 54.0931 | OK |
| clang | LLVM | -O2 | 2.0476 | 51.1437 | OK |
| clang | LLVM | -O3 | 2.0862 | 51.3251 | OK |
| zig-cc | LLVM | -O3 | 2.8599 | 53.9506 | OK |
| tcc | - | (default) | 0.0207 | 147.9381 | OK |
| cproc | QBE | (default) | 0.203 | 88.9362 | OK |
| antcc | - | -O0 | 0.0702 | 131.1418 | OK |
| antcc | - | -O3 | 0.0854 | 74.2476 | OK |
| madc | MIR | -O0 | 0.5879 | 120.9439 | OK |
| madc | MIR | -O1 | 0.6291 | 105.7785 | OK |
| madc | MIR | -O2 | 0.7044 | 95.2722 | OK |
| mucc | - | -O0 | 0.1734 | 223.8188 | OK |
| cppgm | GM | -O0 | 0.592 | 121.0911 | OK |
| cppgm | GM | -O1 | 0.6287 | 105.8937 | OK |
| cppgm | GM | -O2 | 0.7068 | 95.2613 | OK |
| cppgm | GM | -O3 | 0.7066 | 95.0949 | OK |
| mycc | LLVM | (default) | 1.2636 | 60.8043 | OK |
| mycc | LLVM | --final | 2.4311 | 51.9936 | OK |
| mycc | C | (default) | 2.3307 | 57.0358 | OK |
| mycc | C | --final | 3.0653 | 52.6486 | OK |
| mycc | GM | (default) | 0.6971 | 57.4345 | OK |
| mycc | GM | --final | 0.81 | 52.6276 | OK |
| mycc | QBE | (default) | 0.6802 | 77.5978 | OK |

![plot](https://raw.githubusercontent.com/kostya/stuff/master/index-c/experiment2_plot1.png)

# Related

* [index-cpp](https://github.com/kostya/index-cpp) - A single-file, self-contained stress test for C++ compilers and optimizers.
* [LangArena](https://github.com/kostya/LangArena) - LangArena: Programming Language Evaluation Suite.


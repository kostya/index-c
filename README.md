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

# Experiment: GCC and Clang performance over 10 years. Version 3.

I measured runtime, compilation time, and binary size for `index.c` across GCC 4–16 and Clang 3–23, using the flags `-O0`, `-O1`, `-O2`, `-O3`, `-Os`, `-Oz`, `-Ofast`, and `-O3 -march=native`.

Rerun on 2026-10-29. The first post was deleted because I was doing several measurements and the results were jumping around, so I started doubting them and deleted the post. Now I've done several runs and averaged the results (shown as spread bands on the graphs). I also tuned the configuration for `index.c` so it wouldn't be too favored toward Clang. In the previous run, Clang was much faster on the `Base64::Encode` test - its baseline was 1s on Clang but 8s on GCC, which hurt GCC a lot. Now this test is calibrated against GCC 1s, and the summary time no longer has a big gap against Clang because of a single test.

Keep in mind that these results are specific to my machine:

- **CPU:** Ryzen 3800X
- **RAM:** 78 GB DDR4-3200
- **OS:** Linux arch 7.2.7
- **Docker:** 29.8.1

They will likely differ under other conditions, so I wouldn't recommend treating them as verified or official, or citing them in any source. This is just my experiment; whether to trust it is up to you. You can run the script below to reproduce it - it requires only Linux, Docker, and Ruby.

The benchmarks are run using Docker, with images pulled from Docker Hub. This can introduce uncertainty into the results, since each image may contain its own version of glibc and other unknowns.

**GCC:** 4.9.4, 5.5.0, 6.5.0, 7.5.0, 8.5.0, 9.5.0, 10.5.0, 11.5.0, 12.5.0, 13.5.0, 14.4.0, 15.3.0, 16.2.0.

**Clang:** 3.9.0, 4.0.0, 5.0.2, 6.0.1, 7.0.1, 8.0.0, 9.0.0, 10.0.1, 11.1.0, 12.0.1, 13.0.1, 14.0.6, 15.0.7, 16.0.6, 17.0.6, 18.1.8, 19.1.7, 20.1.8, 21.1.8, 22.1.8, 23.1.4.

### Results for `-O1`, `-O2`, `-O3`

![plot](https://raw.githubusercontent.com/kostya/stuff/master/index-c/plot1_runtime.png)

![plot](https://raw.githubusercontent.com/kostya/stuff/master/index-c/plot1_compile.png)

Regarding the slowdown in GCC compile time (in `-O2`/`-O3` mode), it is clearly reproducible with two simple commands (using the official GCC images):

```
docker run --rm -v `pwd`:/src -w /src gcc:7 bash -c 'time gcc /src/index.c -O3 -lm -lpthread -o /tmp/index'
```
real  0m2.660s

```
docker run --rm -v `pwd`:/src -w /src gcc:16 bash -c 'time gcc /src/index.c -O3 -lm -lpthread -o /tmp/index'
```
real  0m4.805s

This is also confirmed by this source: https://github.com/lac-dcc/BenchGen/wiki/Comparing-gcc-versions#1 `Comparing version 5 and version 14, there is an increase of about 42% over a span of 9 years.`

It's also worth noting that `index.c` is a large (500 KB) amalgamated file. GCC may struggle with this kind of use case, which is actually not typical. This was not tested on smaller files - that is material for another experiment.

### Evolution plot for `-O2`

![plot](https://raw.githubusercontent.com/kostya/stuff/master/index-c/plot6.png)

### No optimizations: `-O0`

![plot](https://raw.githubusercontent.com/kostya/stuff/master/index-c/plot2_runtime.png)

![plot](https://raw.githubusercontent.com/kostya/stuff/master/index-c/plot2_compile.png)

### Compile memory, Mb

![plot](https://raw.githubusercontent.com/kostya/stuff/master/index-c/plot7_compile_mem.png)

There is a large jump in memory usage for Clang between 2020-03-24 and 2020-10-12, and it coincided with a change in the source of the images from kunotoki to silkeh (kunitoki/clang-10 -> silkeh/clang:11), https://github.com/kostya/index-c/blob/master/experiment/run.rb#L29. It's possible this is related to the flags these images were built with rather than a degradation in Clang itself.

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

# Related

* [index-cpp](https://github.com/kostya/index-cpp) - A single-file, self-contained stress test for C++ compilers and optimizers.
* [LangArena](https://github.com/kostya/LangArena) - LangArena: Programming Language Evaluation Suite.


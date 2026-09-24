# C Stress Test

This is an amalgamated/simplified version of the LangArena benchmark
(https://github.com/kostya/LangArena) for the C language.

LangArena: A collection of 50 tasks across 26 languages - complex,
non-synthetic, and inspired by real-world problems (JSON, Base64, CSV,
neural networks, compression, maze A*, graph algorithms, sorting, hashing,
interpreters, parallel matmul, and more).

Fully "all in" - it has no external dependencies beyond the standard
libraries. That is, this file is completely self-contained and can be used
as a stress test for C compilers on its own, as well as for the optimizer.

It also includes the config directly in this file.

All parameters have been tuned so that each test runs for roughly 1 second
on my machine. That is, the total time is approximately 50 seconds (but
results may differ on other hardware).

### Build and run:

    gcc -Wno-format index.c -O3 -lm -o ./index
    ./index

## GCC and Clang: 10 years history

To run test:

    ruby run.rb 

Results in [history.js](https://github.com/kostya/index-c/blob/master/history.js)

![plot](plot_runtime.png)

![plot](plot_compile.png)

Conclusion for GCC -O3 over 12 years:
* Runtime sped up by 5% (62.22s -> 59.13s). 
* Compile time slowed down by 66% (2.89s -> 4.82s).

Conclusion for Clang -O3 over 10 years:
* Runtime sped up by 8% (56.41s -> 52.15s).
* Compile time sped up by 4% (2.29s -> 2.19s).

All benchmarks were run 24 September 2026 on Ryzen 3800X, Ubuntu 24.04 (kernel 7.0.0-30-generic), Docker 28.3.2.


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

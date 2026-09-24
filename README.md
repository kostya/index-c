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

# ~~How GCC and Clang performance changed over 10 years: my experiment~~

People are writing to me and saying that the slowdown GCC compilation doesn't reproduce - it seems to be a specific bug with my platform/Docker or some other conditions and instability. I decided to delete the post so as not to mislead people. I apologize for the trouble.

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

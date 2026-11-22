# Test different C compilers on index.c
---

### Build image

```
cd experiment2
docker build -t index-c-experiment2 .
```

### Show versions

```
docker run --rm -it -w /src/experiment2 -v `pwd`/..:/src index-c-experiment2
ruby versions.rb
```

```
clang     : clang version 23.1.5
gcc       : gcc (GCC) 16.2.1 20260810
tcc       : tcc version 0.9.28rc 2026-01-17 makepkg@b39cbc70 (x86_64 Linux)
zig       : 0.17.1
mycc      : mycc 0.13.0-a00f054 c11-like compiler (backend: unknown) (https://github.com/kostya/myc)
antcc     : antcc version 0.4.7-4-gcd7c80c
madc      : madc 0.106.1
```

### Run
```
docker run --rm -it -w /src/experiment2 -v `pwd`/..:/src index-c-experiment2
ruby run.rb
```













Show versions:
```
docker run --rm myc-benchmarks sh -c 'for t in clang gcc cproc qbe cppgm myc-llvm myc-qbe myc-c myc-gm; do printf "%s: " "$t"; command -v "$t" >/dev/null 2>&1 && { "$t" --version 2>&1 | head -n1 || "$t" -v 2>&1 | head -n1; } || echo "not found"; done'
```

```
clang: clang version 23.1.5
gcc: gcc (GCC) 16.2.1 20260810
cppgm: cppgm 0.10.3 (host configuration: g++)
myc-llvm: myc 0.13.0-a00f054 LLVM 23.1.5 (https://github.com/kostya/myc)
myc-qbe: myc 0.13.0-a00f054 QBE fab6d40 (https://github.com/kostya/myc)
myc-c: myc 0.13.0-a00f054 C(clang version 23.1.5) (https://github.com/kostya/myc)
myc-gm: myc 0.13.0-a00f054 GM v0.10.3 (https://github.com/kostya/myc)
```

# 1. C compilers on index-c: (clang, gcc, mycc, cproc)

Running on [index-c](https://github.com/kostya/index-c) benchmark.

```
docker run --rm -it -w /src -v `pwd`:/src myc-benchmarks
cd 1.index-c
ruby run.rb
```
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

### Run
```
docker run --rm -it -w /src/experiment2 -v `pwd`/..:/src index-c-experiment2
ruby run.rb
```

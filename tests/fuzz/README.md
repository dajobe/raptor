# Fuzz Targets

This directory contains libFuzzer-style harnesses for Raptor.

Developer: Fuzzing
------------------

Requirements
- Clang with libFuzzer and sanitizers (ASan/UBSan).
- macOS: use Homebrew LLVM clang (Apple CLT/Xcode clang does not ship libFuzzer).
- Linux: use a full LLVM toolchain (not a stripped distro clang).

Build (Autotools)
-----------------

Fuzz targets build only when the fuzzer flags **link successfully**.

```sh
./configure --enable-fuzzing \
  CC=/opt/homebrew/opt/llvm/bin/clang \
  FUZZ_CFLAGS="-fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer -O1 -g" \
  FUZZ_LDFLAGS="-fsanitize=fuzzer,address,undefined -Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++ /opt/homebrew/opt/llvm/lib/c++/libc++.1.dylib /opt/homebrew/opt/llvm/lib/c++/libc++abi.1.dylib"
make -C tests/fuzz
```

Build (CMake)
-------------

```sh
cmake -S . -B build-fuzz \
  -DRAPTOR_ENABLE_FUZZING=ON \
  -DRAPTOR_FUZZER_FLAGS="-fsanitize=fuzzer,address,undefined -fno-omit-frame-pointer -O1 -g"
cmake --build build-fuzz
```

Run
---

```sh
mkdir -p tests/fuzz/artifacts
./tests/fuzz/fuzz_turtle \
  -max_total_time=300 \
  -timeout=10 \
  -jobs=8 -workers=8 \
  -rss_limit_mb=4096 \
  -use_value_profile=1 \
  -print_final_stats=1 \
  -artifact_prefix=tests/fuzz/artifacts/ \
  tests/turtle
```

Useful flags
- `-max_total_time=SECONDS`
- `-timeout=SECONDS`
- `-jobs=N -workers=N`
- `-rss_limit_mb=4096`
- `-print_final_stats=1`

Seed corpora
-----------

Use existing tests as seeds:

- Turtle: `tests/turtle/*.ttl`
- N-Triples: `tests/ntriples/*.nt` (and `tests/ntriples-2013/*.nt`)
- RDF/XML: `tests/rdfxml/*.rdf`
- RDFa: `tests/rdfa/*.html` (if present)
- RSS: `tests/feeds/*.rss` or `.xml`
- JSON: `tests/json/*.json`

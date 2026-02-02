# Fuzz Targets

This directory contains libFuzzer-style harnesses for Raptor parsers.

## Harnesses

| Target        | Parser | Seed corpus          |
|---------------|--------|----------------------|
| `fuzz_turtle` | Turtle | `tests/turtle/*.ttl` |

Future harnesses can be added for other parsers (N-Triples, RDF/XML,
RDFa, JSON, RSS Tag Soup) following the same pattern.

## Requirements

- Clang with libFuzzer and sanitizers (ASan/UBSan).
- macOS: use Homebrew LLVM clang (Apple CLT/Xcode clang does not
  ship libFuzzer).
- Linux: use a full LLVM toolchain (not a stripped distro clang).

## Build (Autotools)

Fuzz targets build only when the fuzzer flags **link successfully**.

```sh
./configure --enable-fuzzing \
  CC=/opt/homebrew/opt/llvm/bin/clang \
  FUZZ_CFLAGS="-fsanitize=fuzzer,address,undefined \
    -fno-omit-frame-pointer -O1 -g" \
  FUZZ_LDFLAGS="-fsanitize=fuzzer,address,undefined \
    -Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++ \
    /opt/homebrew/opt/llvm/lib/c++/libc++.1.dylib \
    /opt/homebrew/opt/llvm/lib/c++/libc++abi.1.dylib"
make -C tests/fuzz
```

## Build (CMake)

```sh
cmake -S . -B build-fuzz \
  -DRAPTOR_ENABLE_FUZZING=ON \
  -DRAPTOR_FUZZER_FLAGS="-fsanitize=fuzzer,address,undefined \
    -fno-omit-frame-pointer -O1 -g"
cmake --build build-fuzz
```

## Run

Use the make target (Autotools):

```sh
make -C tests/fuzz fuzz-turtle
```

Or run directly:

```sh
mkdir -p tests/fuzz/artifacts tests/fuzz/corpus/turtle
./tests/fuzz/fuzz_turtle \
  -max_total_time=300 \
  -timeout=10 \
  -jobs=8 -workers=8 \
  -rss_limit_mb=4096 \
  -use_value_profile=1 \
  -print_final_stats=1 \
  -artifact_prefix=tests/fuzz/artifacts/ \
  tests/fuzz/corpus/turtle \
  tests/turtle
```

## Run tips

- libFuzzer writes new corpus entries to the **first** corpus directory
  listed.  Keep `tests/fuzz/corpus/<parser>` first so new inputs do
  not pollute the test suite sources.
- Reproduce a single crash:
  `./tests/fuzz/fuzz_turtle tests/fuzz/artifacts/crash-*`
- Stop on first ASan report:
  `ASAN_OPTIONS=halt_on_error=1:print_stacktrace=1`
- macOS multi-worker note: if you see
  `dyld: Library not loaded: /usr/local/lib/libraptor2.0.dylib`,
  refresh the install name to use the build tree rpath:

```sh
install_name_tool -id @rpath/libraptor2.0.dylib src/.libs/libraptor2.0.dylib
install_name_tool -change /usr/local/lib/libraptor2.0.dylib \
  @rpath/libraptor2.0.dylib tests/fuzz/.libs/fuzz_turtle
```

## Useful libFuzzer flags

- `-max_total_time=SECONDS` -- wall-clock limit for the run
- `-timeout=SECONDS` -- per-input timeout
- `-jobs=N -workers=N` -- parallel workers
- `-rss_limit_mb=4096` -- RSS memory limit
- `-print_final_stats=1` -- print coverage stats at exit

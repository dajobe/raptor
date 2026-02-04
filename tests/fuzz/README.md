# Fuzz Targets

This directory contains libFuzzer-style harnesses for Raptor parsers.

## Harnesses

| Target          | Parser    |
|-----------------|-----------|
| `fuzz_turtle`   | Turtle    |
| `fuzz_ntriples` | N-Triples |
| `fuzz_rdfxml`   | RDF/XML   |

Seed corpora and corpus directories per target are configured in
`Makefile.am` (see the `fuzz-<name>` make targets).

All targets are compiled from a single generic harness
(`fuzz_parser.c`) with `-DFUZZ_PARSER_NAME=\"<name>\"`.  To add a
new parser, add a build target with the appropriate define and a
seed corpus — no new source file is needed.

Future targets: RDFa, JSON, RSS Tag Soup.

## Requirements

- Clang with libFuzzer (compiler-rt) and sanitizers (ASan/UBSan).

### Linux

Install clang and the compiler-rt fuzzer runtime.  Most
distributions ship these in their LLVM packages:

- **Debian/Ubuntu**: `apt install clang libfuzzer-<VERSION>-dev`
  (e.g. `libfuzzer-17-dev`)
- **Fedora**: `dnf install clang compiler-rt`
- **Gentoo**: `emerge sys-devel/clang` (ensure the `compiler-rt`
  package is also installed)

Verify that libFuzzer links:

```sh
echo '#include <stdint.h>
#include <stddef.h>
int LLVMFuzzerTestOneInput(const uint8_t *d, size_t s) { return 0; }
' > /tmp/ftest.c && clang -fsanitize=fuzzer -o /tmp/ftest /tmp/ftest.c \
  && echo OK || echo FAILED
```

### macOS

Apple Xcode/CLT clang does not ship libFuzzer.  Install LLVM via
Homebrew and use its clang:

```sh
brew install llvm
export CC=/opt/homebrew/opt/llvm/bin/clang
```

## Build (Autotools)

Fuzz targets build only when the fuzzer flags **link successfully**.

### Linux

```sh
./configure --enable-fuzzing \
  CC=clang \
  FUZZ_CFLAGS="-fsanitize=fuzzer,address,undefined \
    -fno-omit-frame-pointer -O1 -g" \
  FUZZ_LDFLAGS="-fsanitize=fuzzer,address,undefined"
make -C tests/fuzz
```

### macOS (Homebrew LLVM)

macOS needs the Homebrew LLVM clang and explicit C++ runtime linking:

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

### Linux

```sh
cmake -S . -B build-fuzz \
  -DCMAKE_C_COMPILER=clang \
  -DRAPTOR_ENABLE_FUZZING=ON \
  -DRAPTOR_FUZZER_FLAGS="-fsanitize=fuzzer,address,undefined \
    -fno-omit-frame-pointer -O1 -g"
cmake --build build-fuzz
```

### macOS (Homebrew LLVM)

```sh
cmake -S . -B build-fuzz \
  -DCMAKE_C_COMPILER=/opt/homebrew/opt/llvm/bin/clang \
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

The `fuzz-ntriples` and `fuzz-rdfxml` targets also use dictionaries by
default (see `tests/fuzz/dict/*.dict`).

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
  -dict=tests/fuzz/dict/turtle.dict \
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
- macOS note: if you see
  `dyld: Library not loaded: /usr/local/lib/libraptor2.0.dylib`,
  refresh the install name to use the build tree rpath for all fuzzers:

```sh
install_name_tool -id @rpath/libraptor2.0.dylib src/.libs/libraptor2.0.dylib
install_name_tool -change /usr/local/lib/libraptor2.0.dylib \
  @rpath/libraptor2.0.dylib tests/fuzz/.libs/fuzz_turtle
install_name_tool -change /usr/local/lib/libraptor2.0.dylib \
  @rpath/libraptor2.0.dylib tests/fuzz/.libs/fuzz_ntriples
install_name_tool -change /usr/local/lib/libraptor2.0.dylib \
  @rpath/libraptor2.0.dylib tests/fuzz/.libs/fuzz_rdfxml
```

## Corpus directories

libFuzzer writes discovered inputs to `tests/fuzz/corpus/<parser>/`.
These directories are gitignored and grow across runs.  Keeping them
between sessions gives better coverage on subsequent runs since the
fuzzer starts from previously discovered inputs rather than just the
seed files.

To reclaim disk space:

```sh
rm -rf tests/fuzz/corpus/*/
```

## Dictionary generation

Regenerate dictionary files from the corpora and built-in token lists:

```sh
python3 scripts/generate_fuzz_dicts.py
black scripts/generate_fuzz_dicts.py
```

## Adding a new parser

1. Find the raptor parser name string (e.g. `"rdfxml"`) from the
   parser registration in `src/`.
2. Add build targets in `Makefile.am` and `CMakeLists.txt` using
   the existing entries as a template.
3. Add a `fuzz-<name>` make target pointing at the seed corpus
   directories.
4. Add the binary name to `.gitignore`.
5. Update the harnesses table above.

## Useful libFuzzer flags

- `-max_total_time=SECONDS` -- wall-clock limit for the run
- `-timeout=SECONDS` -- per-input timeout
- `-jobs=N -workers=N` -- parallel workers
- `-rss_limit_mb=4096` -- RSS memory limit
- `-print_final_stats=1` -- print coverage stats at exit

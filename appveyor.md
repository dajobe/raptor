# AppVeyor Windows builds

## Status

The AppVeyor build is **not yet working**.

The recent repair work has moved the Windows build from failing during
dependency installation to compiling Raptor and its test programs, but no
recent AppVeyor build has completed successfully. The latest completed build
was build 50 at commit `59ca2044`. It failed during dependency setup, before
CMake configuration, because the worker could not connect to
`https://cygwin.com/setup-x86_64.exe`.

Build 50 included both the vcpkg cache configuration from `54eb078c` and the
`RAPTOR_WWW=none` setting from `59ca2044`. It successfully saved the vcpkg cache
after the failure, but did not reach CMake and therefore did not test the WWW
setting.

## Current configuration

The current configuration:

- uses the Visual Studio 2022 AppVeyor image and generates 64-bit Visual Studio
  17 2022 projects;
- initializes the `libfsp` submodule;
- uses CMake to generate `parsedate.c`, the Turtle parser, and the Turtle lexer;
- uses the native Windows Python found by CMake for the post-processing scripts;
- installs 64-bit Cygwin into a job-local directory solely to provide Bison and
  Flex;
- installs `libxml2:x64-windows` with the AppVeyor-provided vcpkg;
- configures both static and shared builds with the vcpkg CMake toolchain;
- disables Raptor's built-in WWW implementation with `RAPTOR_WWW=none`; and
- builds and installs the static tree before attempting the shared tree.

The vcpkg installed tree is configured as an AppVeyor build cache. Saving the
cache after a failed build is enabled. Build 50 created and uploaded an
8,406,250-byte cache entry successfully. Cache restoration has not yet been
observed.

## What has been established

The recent builds established the following:

- The old 32-bit Cygwin installer and package command are obsolete.
- A fresh 64-bit Cygwin installation can provide Bison 3.8.2 and Flex 2.6.4.
  Those versions satisfy Raptor's CMake requirements.
- The Visual Studio 2022 image supplies a sufficiently new CMake. The older
  image supplied CMake 3.16.2, while Raptor requires at least CMake 3.17.
- Cygwin Python is not suitable for CMake custom commands containing native
  Windows paths. Native Windows Python 3.14 was found automatically and
  successfully ran the generation scripts.
- CMake can generate all required parser and lexer sources on the Windows
  worker.
- The embedded `libfsp` sources need the generated `raptor_config.h` on Windows,
  not the removed `win32_raptor_config.h`. This was corrected in `e761965a`.
- vcpkg's libxml2 is found by CMake and is sufficient to compile Raptor's XML
  support.
- vcpkg's current libxml2 does not provide the `xmlNanoHTTP*` symbols used by
  `raptor_www_libxml.c`. Build 49 therefore compiled but failed when linking
  the test executables.
- `APPVEYOR_SAVE_CACHE_ON_ERROR=true` works for this project. Build 50 saved the
  vcpkg installed tree after its dependency-stage failure.
- Downloading the Cygwin installer is a separate availability risk. Build 50
  failed when `Invoke-WebRequest` could not connect to `cygwin.com`.

Disabling `RAPTOR_WWW` is intended to avoid that final link failure without
removing libxml2-backed XML parsing. This has not yet been verified in
AppVeyor.

## Recent build history

- Build 42, `27ebb513`, was cancelled after beginning the switch to 64-bit
  Cygwin and CMake-managed generation.
- Build 43, `9c9013f2`, failed because `C:/cygwin64/bin/python3` was not
  installed.
- Build 44, `7aed16ed`, failed because the revised Cygwin package command still
  did not provide `python3`.
- Build 45, `df974fe7`, failed after upgrading the shared Cygwin installation
  downloaded many unrelated packages but still did not provide `python3`.
- Build 46, `70b4fee1`, failed because the isolated Cygwin environment worked
  but the image had CMake 3.16.2.
- Build 47, `c9e40d74`, failed because Cygwin Python could not open a generation
  script expressed as a native Windows path.
- Build 48, `15d1f5e2`, failed after generation and compilation started because
  embedded libfsp included the nonexistent `win32_raptor_config.h`.
- Build 49, `58a7d6e1`, failed after libxml2 support compiled because the static
  test programs could not link the unavailable `xmlNanoHTTP*` API.
- Build 50, `59ca2044`, completed a cold vcpkg installation in about 8.2
  minutes, then failed because the Cygwin installer download could not connect.
  AppVeyor successfully saved the vcpkg cache during finalization.

The Git history also shows the original AppVeyor configuration was introduced
in 2014, adjusted to let CMake generate parser sources in 2015, and updated for
real Bison input file names in 2020. It then received no changes until the 2026
repair series above.

## Known issues and limitations

1. There is no successful build of the current configuration.
2. The `RAPTOR_WWW=none` workaround has not been tested on AppVeyor. It also
   means this Windows build has no built-in HTTP transport.
3. The vcpkg cache has been saved but has not been proven to restore. A later
   build is needed to demonstrate a cache hit.
4. A cold vcpkg installation is expensive. In build 50 it took about 8.2
   minutes, including about 7.3 minutes for libiconv.
5. Cygwin is still downloaded and installed for every clean worker to obtain
   only Bison and Flex. Its setup reports missing fresh-install metadata files
   before continuing. Build 50 also showed that downloading the installer from
   `cygwin.com` is a single point of failure.
6. Only the static configuration has reached the link stage. The shared build
   and both install steps have not completed on the current toolchain.
7. The configuration has no `test_script` and does not run CTest. Building the
   test executables is not the same as executing the test suite.
8. `appveyor.yml` restricts automatic builds to a branch named `appveyor`, but
   the recent manual builds were run against `master`. The intended trigger and
   branch policy need to be clarified.
9. The job installs libxml2 but not curl, libxslt, or YAJL. Consequently the
   current configuration has no curl WWW backend, GRDDL support, or YAJL JSON
   support. These should be deliberate feature decisions rather than accidental
   omissions.

## Next checkpoint

The next useful checkpoint is one repeat build of `59ca2044`. Its purpose
should be limited to answering these questions:

1. Does AppVeyor restore the saved vcpkg cache and avoid rebuilding libiconv?
2. Can the worker download and install the Cygwin dependencies?
3. Does `RAPTOR_WWW=none` allow the static build to link and install?
4. Does the shared build compile, link, and install?

Only after those are answered should the configuration be expanded or further
dependencies be added.

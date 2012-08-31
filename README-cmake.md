This file is based on the email that Daniel Richard G. sent to
redland-dev:

    http://lists.librdf.org/pipermail/redland-dev/2012-July/002502.html

I lightly edited the email to fit into 80 chars and make more it
markdowny and turn it into notes.

-- Dave

----------------------------------------------------------------------
Date: Thu, 5 Jul 2012 14:56:01 -0400
From: Daniel Richard G.
To: redland-dev

Hello list,

I've implemented support for building the Raptor library using the
CMake build configuration tool. This is intended not to replace the
existing GNU-Autotools-based configuration / build framework, but to
provide a better solution for building Raptor on Windows (and
potentially other platforms) than hand-maintained project files for
various popular IDEs.

* [http://cmake.org/](CMake)

* [http://en.wikipedia.org/wiki/CMake](CMake on Wikipedia)

There are several reasons why I chose CMake for this:

* It can generate project / solution / workspace files for basically
  every version of Visual Studio in existence, from a common set of
  definitions

* Likewise, it can generate project files for less-common IDEs (e.g.
  CodeBlocks, Apple Xcode) and makefile-trees for NMake, Borland,
  MSYS...

* A friendly GUI frontend is provided on Windows, great for IDE users
  who like to click on things

* CMake doesn't neglect to support Linux / Unix, of course; even
  black-sheep systems like AIX are covered

* The tool is actively maintained and developed by the folks at Kitware

* The KDE folks moved whole-hog from Autotools to CMake due to its
  solid support for Windows and popular IDEs, and while I certainly
  wouldn't advocate a CMake-only zeitgeist, it certainly speaks to
  their confidence in the tool

* Of course: CMake is free software, distributed under the
  three-clause BSD license

* Teragram and I have used CMake extensively for the purpose of
  facilitating Windows builds of primarily Autotools-based projects,
  and so my own exerience has borne out the strengths of this
  approach.


That's not to say, of course, that the tool is perfect:

* The syntax and naming conventions used in the CMake scripting
  language and standard modules are more in line with Windows culture
  than Unix (ALL_CAPS, semicolon separators and CamelCasing are in
  abundance)

* Some operations, like setting predefined compiler flags, are
  needlessly harder to do compared to Autoconf (where you can just
  e.g. assign to CFLAGS)

* When CMake generates makefiles, they make the ones produced by
  Automake look simple and elegant by comparison :> You definitely
  get more of an IDE-like experience when building with these, which
  some folks may like, but I don't care for at all.


Nevertheless, I consider CMake's strengths to outweigh its
weaknesses. I myself am as much an Autotools-alternative skeptic as
anyone, and tend to look leerily at all the ones that have come
along---especially when I've no choice but to deal with them
(e.g. SCons in NSIS). But CMake not only stands strong where
Autotools is weak (support for non-Cygwin / MSYS Windows
environments, support for IDEs), it does so in a fully general,
polished, and consistent way. This is the one that, in my view, has
risen above the pack.

All that said, then, I'll go on to the particulars of this CMake
implementation for Raptor. (Everything is in the attached patch,
against git master.)

This turned out to be a fairly complex project, because the Raptor
library has so many features that can be enabled / disabled /
configured.  These are not merely controlled by #define'ing
or #undefing cpp symbols; object files also need to be added or
removed, as well as associated third-party library
dependencies. Plus, the library conforms to various potential quirks
in LibXML2, which need to be checked for at configure time. This
complexity may be seen mostly in the top-level CMakeLists.txt file
and src/CMakeLists.txt.

The `win32_raptor_config.h` header is no longer used; this is replaced
by the more general `raptor_config_cmake.h.in`, which CMake
instantiates with configuration-specific values much as Autoconf
instantiates `raptor_config.h.in`. Rather than remove the
`#include <win32_raptor_config.h>` directive from numerous files,
however, I added an `#if 0` block to the header to make it a no-op
(to keep an already large patch from becoming even larger).

In addition to reproducing the library build in CMake, I've also
reproduced most of the test suite. Of course, the test suite is
fairly extensive, and consists of numerous similar invocations of
rapper and rdfdiff; maintaining all of these in Automake is enough of
a task already without the extra work of maintaining it in CMake. So
I opted for an approach wherein the CMake test definitions are
generated as a side effect of the shell code that drives the tests in
Automake.

The patch contains `tests/*/CMakeLists.txt`, of course, but it also
contains changes to the associated `Makefile.am` files that write out
the bulk of the CMake script to `CMakeTests.txt` (filename is
arbitrary; `make clean` deletes it). The intent is not full-auto
generation of the `CMakeLists.txt` files, but to make most of the work
in maintaining them a matter of cut-and-paste. (It wouldn't take much
more to enable full-auto generation, but I think there is value in
having the maintainer at least eyeball what's going in.)

The CMake-based test suite does have a few shortcomings compared to
the Automake-based one, and will need further refinement:

* Tests that compare output to a reference do not check for file
  equality as a way of avoiding the use of rdfdiff. This is a problem
  because rdfdiff currently blows up on certain inputs (e.g. test
  0176 in rdfa11).

* That would be easier to resolve if it weren't for the issue of
  comparing CRLF output from rapper on Windows to LF reference
  files. CMake has built-in functionality to compare files, but as
  currently implemented, it is basically a cross-platform
  cmp(1) --- there's no way to see past differences in EOL
  convention. I've filed a feature request on this...
  [http://public.kitware.com/Bug/view.php?id=13007](CMake Isue 13007)
  ...but for now, I'm using CMake's `CONFIGURE_FILE()` to normalize
  line endings on the output files before doing the comparison.

* `tests/turtle/CMakeLists.txt` has yet to be written, as the
  exit-status logic there is a bit more involved than the other test
  sections.

* There is some awkwardness on Windows, when the rapper and rdfdiff
  binaries depend on third-party DLLs (e.g. LibXML2). A correctly-set
  PATH allows the DLLs to be found, but Visual Studio isn't terribly
  straightforward about how to set PATH when running a program, and
  (IIRC) the failure mode was not even obvious to begin with. I've
  addressed this, if a little ham-handedly, by enabling the test
  suite only when building Raptor with a makefile tree.


Other caveats of this CMake implementation:

* This build framework is not enough to produce a Raptor DLL. There
  are issues regarding DLL-export linkage of various functions in
  `raptor_internal.h` and `turtle_common.h` that need
  addressing. I'll bring those up here on the list once the CMake
  stuff is hashed out.

* Support for JSON --- and more specifically, the YAJL library --- is
  penciled in, but not yet working or tested. (I have no experience
  with this library, let alone on Windows.)

* Generation of `turtle_lexer.c`, `turtle_parser.c` and such is not
  implemented at all. This can be added, but my working premise is
  that the CMake build framework is meant for library users, not
  developers.


If you would like to kick the tires of this CMake implementation,
here are some steps to get you started:

1. Apply my patch to a copy of Raptor's git master source

2. Run `./autogen.sh`, `./configure` and `make dist`

3. Unpack the resulting dist tarball somewhere

4. Download and install the CMake tool

5. (Linux) Create a new, empty build directory, and from there,
   invoke

     $ cmake /path/to/raptor2-dist-src

   This is the equivalent of running plain `./configure`, with
   default values for everything. Provided that you have all the
   requisite libraries installed, this should produce a makefile
   tree.

5. (Windows) Run the `cmake-gui` application, set the source and
   build paths at the top (the latter should be a new, empty
   directory) and hit Configure. Select an appropriate "generator"
   (this is where you choose the specific IDE or other build system
   you want), then hit Finish. Allow CMake to run the configuration
   checks, and if these succeed, hit Generate. Once the generation
   process is finished, you may close CMake and use the
   newly-generated build system.

6. If you are building with makefiles, the test suite is invoked with
   the "test" target, not "check".

Questions and comments on this implementation are welcome; I'll do my
best to answer any. This framework addresses a difficulty that
Teragram has had with this library, and I hope it will do the same
for others here.


--Daniel

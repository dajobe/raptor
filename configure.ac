dnl -*- Mode: autoconf -*-
dnl
dnl configure.ac - autoconf file for Raptor
dnl (Process this file with autoconf to produce a configure script.)
dnl
dnl Copyright (C) 2000-2011 David Beckett http://www.dajobe.org/
dnl Copyright (C) 2000-2005 University of Bristol, UK http://www.bristol.ac.uk/
dnl 
dnl This package is Free Software and part of Redland http://librdf.org/
dnl 
dnl It is licensed under the following three licenses as alternatives:
dnl   1. GNU Lesser General Public License (LGPL) V2.1 or any newer version
dnl   2. GNU General Public License (GPL) V2 or any newer version
dnl   3. Apache License, V2.0 or any newer version
dnl 
dnl You may not use this file except in compliance with at least one of
dnl the above three licenses.
dnl 
dnl See LICENSE.html or LICENSE.txt at the top of this package for the
dnl complete terms and further detail along with the license texts for
dnl the licenses in COPYING.LIB, COPYING and LICENSE-2.0.txt respectively.
dnl 
dnl 


AC_PREREQ([2.62])
AC_INIT([Raptor RDF Parser and Serializer library], [2.0.15], [http://bugs.librdf.org/], [raptor2])
AC_CONFIG_SRCDIR([src/raptor_general.c])
AC_CONFIG_HEADERS([src/raptor_config.h])
AC_CONFIG_AUX_DIR(build)
AC_CONFIG_MACRO_DIR(build)
AM_INIT_AUTOMAKE([1.11 check-news std-options -Wobsolete -Wportability -Wsyntax -Wunsupported -Wextra-portability])

libxml_min_version=2.6.8

libxslt_min_version=1.0.18

libcurl_min_version=7.12.0
libcurl_min_vernum=071200

AC_REVISION($Revision: $)dnl

AM_MAINTAINER_MODE

release_version=no
AC_ARG_ENABLE(release, [  --enable-release        Turn on optimizations (for maintainer).  ], \
 if test "$enableval" = "yes"; then \
	  release_version=yes
fi;)

AM_CONDITIONAL(RELEASE_VERSION, test $release_version = yes)
if test "$USE_MAINTAINER_MODE" = yes -a $release_version = no; then 
  dnl need to change quotes to allow square brackets
  changequote(<<, >>)dnl
  CFLAGS=`echo $CFLAGS | sed -e "s/-O[A-Za-z0-9]*//"`
  CXXFLAGS=`echo $CXXFLAGS | sed -e "s/-O[A-Za-z0-9]*//"`
  CPPFLAGS=`echo $CPPFLAGS | sed -e "s/-O[A-Za-z0-9]*//"`
  changequote([, ])dnl
fi
RPM_RELEASE=SNAP
if test "$release_version" = "yes"; then
  RPM_RELEASE=1
fi
AC_SUBST(RPM_RELEASE)


dnl Checks for programs.
AC_CANONICAL_HOST
AM_SANITY_CHECK
AM_PROG_AR
AC_PROG_CC
AM_PROG_CC_C_O
AC_MSG_CHECKING(whether $CC is clang)

dnl Initialize libtool
LT_INIT

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

FLEX_MIN_VERSION=2.5.19
FLEX_REC_VERSION=2.5.36

# Do not want AM_PROG_LEX which adds 'missing' to LEX if it's not around
AC_PROG_LEX
AC_MSG_CHECKING(flex)
if test "$USE_MAINTAINER_MODE" = yes; then
  # maintainer mode - flex is required
  if test "X$LEX" = "X:" ; then
    AC_MSG_RESULT(not present)
    AC_MSG_WARN(Please get flex from http://flex.sourceforge.net/)
    AC_MSG_WARN(version $FLEX_MIN_VERSION ($FLEX_REC_VERSION recommended))
    AC_MSG_FAILURE(flex not present)
  fi

  # some kind of lexer is present
  if echo "$LEX" | grep flex >/dev/null 2>&1; then
    # flex is present
    FLEX_VERSION=`$LEX -V 2>&1 | $AWK '{print $2}'`
    FLEX_VERSION_DEC=`echo $FLEX_VERSION | $AWK -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};'`

    FLEX_MIN_VERSION_DEC=`echo $FLEX_MIN_VERSION | $AWK -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};'`

    if test $FLEX_VERSION_DEC -ge $FLEX_MIN_VERSION_DEC; then
      AC_MSG_RESULT($FLEX_VERSION - OK)
    else
      AC_MSG_RESULT(version $FLEX_VERSION - too old)
      AC_MSG_WARN(Please get flex from http://flex.sourceforge.net/)
      AC_MSG_WARN(version $FLEX_MIN_VERSION ($FLEX_REC_VERSION recommended))
      AC_MSG_FAILURE(flex is too old)
    fi
  else
    AC_MSG_RESULT(present - but is not flex)
    AC_MSG_WARN(Please get flex from http://flex.sourceforge.net/)
    AC_MSG_WARN(version $FLEX_MIN_VERSION ($FLEX_REC_VERSION recommended))
    AC_MSG_FAILURE($LEX is not not flex)
  fi
else
  # not maintainer mode; flex is not required
  AC_MSG_RESULT(not present - not required for non maintainer builds)
  LEX="$SHELL $missing_dir/missing flex"
  AC_SUBST(LEX_OUTPUT_ROOT, lex.yy)
  AC_SUBST(LEXLIB, '')
  FLEX_VERSION_DEC=00000
fi
AC_DEFINE_UNQUOTED(FLEX_VERSION_DECIMAL, $FLEX_VERSION_DEC, [Flex version as a decimal])


BISON_MIN_VERSION=3.0.0
AC_CHECK_PROGS(BISON, bison3 bison)
if test "$USE_MAINTAINER_MODE" = yes; then
  AC_MSG_CHECKING(for GNU bison)
  # Match these styles of versions
  # GNU Bison version 1.28
  # bison (GNU Bison) 1.875
dnl need to change quotes to allow square brackets
changequote(<<, >>)dnl
  BISON_VERSION=`$BISON --version 2>&1 | sed -ne 's/^.*GNU Bison[^0-9]*//p'`
changequote([, ])dnl
  if test "X$BISON_VERSION" != X; then
    BISON_VERSION_DEC=`echo $BISON_VERSION | $AWK -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};'`

    BISON_MIN_VERSION_DEC=`echo $BISON_MIN_VERSION | $AWK -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};'`

    if test $BISON_VERSION_DEC -ge $BISON_MIN_VERSION_DEC; then
      AC_MSG_RESULT($BISON_VERSION - OK)
    else
      AC_MSG_RESULT(version $BISON_VERSION is too old)
      AC_MSG_WARN(Please get GNU Bison from http://www.gnu.org/software/bison/)
      AC_MSG_WARN(version $BISON_MIN_VERSION or newer)
      AC_MSG_FAILURE(GNU Bison too old)
    fi
  else
    AC_MSG_FAILURE($BISON is not GNU bison)
  fi
fi
AC_SUBST(BISON)

# Find a tar command for 'make dist'
AC_CHECK_PROGS(TAR, gnutar gtar tar)
AC_CHECK_PROGS(PERL, perl)
# Used in tests/feeds
AC_CHECK_PROGS(JING, jing)

AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)

AC_CHECK_PROGS(RECHO, echo)
RECHO_C=
RECHO_N=
case `$RECHO -n x` in
  -n*)
    case `$RECHO 'xy\c'` in
      *c*)
         ;;
      xy)
         RECHO_C='\c'
         ;;
    esac;;
  *)
    RECHO_N='-n'
    ;;
esac



dnl compiler checks

AC_DEFUN([REDLAND_CC_TRY_FLAG], [
  AC_MSG_CHECKING([whether $CC supports $1])

  ## backup CFLAGS and werror status
  redland_save_CFLAGS="$CFLAGS"
  redland_save_ac_c_werror_flag="${ac_c_werror_flag}"
  AC_LANG_WERROR

  CFLAGS="$CFLAGS $1"
  AC_COMPILE_IFELSE([AC_LANG_SOURCE([ ])], [redland_cc_flag=yes], [redland_cc_flag=no])

  ## restore CFLAGS and werror status
  CFLAGS="$redland_save_CFLAGS"
  ac_c_werror_flag="${redland_save_ac_c_werror_flag}"

  if test "X$redland_cc_flag" = "Xyes"; then
    ifelse([$2], , :, [$2])
  else
    ifelse([$3], , :, [$3])
  fi
  AC_MSG_RESULT($redland_cc_flag)
])

# GCC warning options
# http://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
#
# Too noisy:
#  -Wconversion
#  -Wformat-nonliteral : variables for format strings
#  -Wmissing-field-initializers : too noisy in raptor_rss_common.c
#  -Wsystem-headers   : not debugging system
#  -Wsign-conversion: : many int / unsigned int / size_t
#  -Wunused-parameter : variables can be marked  __attribute__('unused')
#
# Apple gcc specific (probably): -Wshorten-64-to-32
possible_warnings="\
-Wall \
-Wc++-compat \
-Wextra \
-Wunused \
\
-Waggregate-return \
-Wbad-function-cast \
-Wcast-align \
-Wdeclaration-after-statement \
-Wdisabled-optimization \
-Wdiv-by-zero \
-Wendif-labels \
-Werror-implicit-function-declaration \
-Wfloat-equal \
-Wformat-security \
-Wframe-larger-than=4096 \
-Winit-self \
-Winline \
-Wmissing-declarations \
-Wmissing-format-attribute \
-Wmissing-noreturn \
-Wmissing-prototypes \
-Wnested-externs \
-Wold-style-definition \
-Wpacked \
-Wpointer-arith \
-Wredundant-decls \
-Wshadow \
-Wsign-compare \
-Wstrict-prototypes \
-Wswitch-enum \
-Wundef \
-Wunreachable-code \
-Wunsafe-loop-optimizations \
-Wwrite-strings \
\
-Wshorten-64-to-32
\
-Wno-conversion \
-Wno-format-nonliteral \
-Wno-missing-field-initializers \
-Wno-sign-conversion \
-Wno-system-headers \
-Wno-unused-parameter \
-Wswitch-bool \
-Wlogical-not-parentheses \
-Wsizeof-array-argument  \
-Wbool-compare \
-Wc90-c99-compat \
-Wc99-c11-compat \
"

warning_cflags=
if test "$USE_MAINTAINER_MODE" = yes; then
  AC_MSG_CHECKING(for supported $CC warning flags)
  AC_MSG_RESULT($warning_cflags)
  for warning in $possible_warnings; do
    REDLAND_CC_TRY_FLAG([$warning], [warning_cflags="$warning_cflags $warning"])
  done
  AC_MSG_CHECKING($CC supports warning flags)
  AC_MSG_RESULT($warning_cflags)
fi

MAINTAINER_CPPFLAGS="$warning_cflags"


dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(errno.h fcntl.h stdlib.h stddef.h unistd.h string.h limits.h math.h getopt.h sys/stat.h sys/param.h sys/stat.h sys/time.h setjmp.h)
AC_CHECK_FUNCS(stat)
AC_HEADER_TIME
dnl FreeBSD fetch.h needs stdio.h and sys/param.h first
AC_CHECK_HEADERS(fetch.h,,,
  [#include <stdio.h>
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
])



dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_BIGENDIAN
AC_C_INLINE

AC_MSG_CHECKING(whether __FUNCTION__ is available)
AC_COMPILE_IFELSE([AC_LANG_SOURCE([int main() { printf(__FUNCTION__); }])],
    [AC_DEFINE([HAVE___FUNCTION__], [1], [Is __FUNCTION__ available])
     AC_MSG_RESULT(yes)],
    [AC_MSG_RESULT(no)])


dnl need to change quotes to allow square brackets
changequote(<<, >>)dnl
version_major=`echo $VERSION | sed -e 's/^\([^\.]*\)\.\([^\.]*\)\.\(.*\)$/\1/'`
version_minor=`echo $VERSION | sed -e 's/^\([^\.]*\)\.\([^\.]*\)\.\(.*\)$/\2/'`
version_release=`echo $VERSION | sed -e 's/^\([^\.]*\)\.\([^\.]*\)\.\(.*\)$/\3/'`
changequote([, ])dnl

version_decimal=`expr $version_major \* 10000 + $version_minor \* 100 + $version_release`

# The minimum runtime API version that is supported.  MUST be updated at
# an API break as well as changing the libtool version
min_version_decimal=20000

AC_DEFINE_UNQUOTED(RAPTOR_VERSION_MAJOR, $version_major, [Major version number])
AC_DEFINE_UNQUOTED(RAPTOR_VERSION_MINOR, $version_minor, [Minor version number])
AC_DEFINE_UNQUOTED(RAPTOR_VERSION_RELEASE, $version_release, [Release version number])
AC_DEFINE_UNQUOTED(RAPTOR_VERSION_DECIMAL, $version_decimal, [Release version as a decimal])
AC_DEFINE_UNQUOTED(RAPTOR_MIN_VERSION_DECIMAL, $min_version_decimal, [Minimum supported package version])

# for raptor-config.in
RAPTOR_VERSION_MAJOR=$version_major
RAPTOR_VERSION_MINOR=$version_minor
RAPTOR_VERSION_RELEASE=$version_release
RAPTOR_VERSION_DECIMAL=$version_decimal
AC_SUBST(RAPTOR_VERSION)
AC_SUBST(RAPTOR_VERSION_MAJOR)
AC_SUBST(RAPTOR_VERSION_MINOR)
AC_SUBST(RAPTOR_VERSION_RELEASE)
AC_SUBST(RAPTOR_VERSION_DECIMAL)

# Libtool versioning
#
# CURRENT
#      The most recent interface number that this library implements.
# 
# REVISION
#      The implementation number of the CURRENT interface.
# 
# AGE
#      The difference between the newest and oldest interfaces that this
#      library implements.  In other words, the library implements all the
#      interface numbers in the range from number `CURRENT - AGE' to
#      `CURRENT'.
#
# Rules:
#  1. Start with version information of `0:0:0' for each libtool library.
#
#  2. Update the version information only immediately before a public
#     release of your software.  More frequent updates are unnecessary,
#     and only guarantee that the current interface number gets larger
#     faster.
#
#  3. If the library source code has changed at all since the last
#     update, then increment REVISION (`C:R:A' becomes `C:r+1:A').
#
#  4. If any interfaces have been added, removed, or changed since the
#     last update, increment CURRENT, and set REVISION to 0.
#
#  5. If any interfaces have been added since the last public release,
#     then increment AGE.
#
#  6. If any interfaces have been removed since the last public release,
#     then set AGE to 0.
#
# syntax: CURRENT[:REVISION[:AGE]]
RAPTOR_LIBTOOL_VERSION=0:0:0
AC_SUBST(RAPTOR_LIBTOOL_VERSION)


dnl Checks for library functions.
AC_CHECK_FUNCS(gettimeofday getopt getopt_long stricmp strcasecmp vsnprintf isascii setjmp strtok_r qsort_r qsort_s)

dnl librdfa
AM_CONDITIONAL([NEED_STRTOK_R], [test "$ac_cv_func_strtok_r" = "no"])

dnl Check for GNU extension functions
oCPPFLAGS="$CPPFLAGS"
CPPFLAGS="-D_GNU_SOURCE $CPPFLAGS"
AC_CHECK_FUNCS(vasprintf)
CPPFLAGS="$oCPPFLAGS"


AM_CONDITIONAL(STRCASECMP, test $ac_cv_func_strcasecmp = no -a $ac_cv_func_stricmp = no)
AM_CONDITIONAL(GETOPT, test $ac_cv_func_getopt = no -a $ac_cv_func_getopt_long = no)


AC_MSG_CHECKING(whether need to declare optind)
AC_LINK_IFELSE([AC_LANG_PROGRAM([[#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif]], [[int x=optind;]])],
            [AC_MSG_RESULT(no)],
            [AC_DEFINE([NEED_OPTIND_DECLARATION], [1], [need 'extern int optind' declaration?])
            AC_MSG_RESULT(yes)])



if test $ac_cv_func_vsnprintf = yes; then
  AC_MSG_CHECKING([whether vsnprintf has C99 compatible return value])
  AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#include <stdarg.h>

static int is_c99(char *buf, char *s, ...) {
  va_list args;
  int r;
  va_start(args, s);
  r = vsnprintf(buf, buf ? 5 : 0, s, args);
  va_end(args);

  return (r == 7);
}

int main(void) {
  char buffer[32];
  return (is_c99(NULL,   "1234567") ? 0 : 10)
       + (is_c99(buffer, "1234567") ? 0 :  1);
}]])],
    [AC_DEFINE([HAVE_C99_VSNPRINTF], [1], [vsnprintf has C99 compatible return value])
     AC_MSG_RESULT([yes])],
    [AC_MSG_RESULT([no])],
    [AC_DEFINE([CHECK_VSNPRINTF_RUNTIME], [1], [have to check C99 vsnprintf at runtime because cross compiling])])
fi

# Save LIBS 
oLIBS="$LIBS"

RAPTOR_LDFLAGS=

AC_SYS_LARGEFILE


PKG_PROG_PKG_CONFIG

PKG_CONFIG_REQUIRES=

dnl libxml - required
AC_ARG_WITH(xml2-config, [  --with-xml2-config=PATH   Location of libxml xml2-config []], xml2_config="$withval", xml2_config="")

if test "X$xml2_config" != "Xno" ; then
  if test "X$xml2_config" != "X" ; then
    AC_MSG_CHECKING(for $xml2_config)

    if test -x $xml2_config ; then
      XML_CONFIG=$xml2_config
      AC_MSG_RESULT(yes)
    else
      AC_MSG_ERROR([xml2-config not found at specified path $xml2_config])
    fi
  fi
  if test "X$XML_CONFIG" = "X" ; then
    AC_CHECK_PROGS(XML_CONFIG, xml2-config)
  fi
fi

libxml_source=no
if test "X$XML_CONFIG" != "X"; then
  LIBXML_CFLAGS=`$XML_CONFIG --cflags`
  LIBXML_LIBS=`$XML_CONFIG --libs`

  CPPFLAGS="$LIBXML_CFLAGS $CPPFLAGS"
  LIBS="$LIBS $LIBXML_LIBS"
  AC_CHECK_FUNC(xmlCreatePushParserCtxt, have_xmlCreatePushParserCtxt=yes, have_xmlCreatePushParserCtxt=no)

  AC_MSG_CHECKING(for libxml via xml2-config)
  if test $have_xmlCreatePushParserCtxt = yes; then
    libxml_source="xml2-config"
    LIBXML_VERSION=`$XML_CONFIG --version`
  fi
  CPPFLAGS="$oCPPFLAGS"
  LIBS="$oLIBS"
else
  PKG_CHECK_MODULES([LIBXML],[libxml-2.0],[
    LIBXML_VERSION=`$PKG_CONFIG libxml-2.0 --modversion`
    libxml_source="pkg-config"
  ], [:])
  AC_MSG_CHECKING(for libxml via pkg-config)
fi

if test "$libxml_source" != "no"; then
  AC_MSG_RESULT(yes - $LIBXML_VERSION)
else
  AC_MSG_RESULT(no - not found)
fi


dnl xslt
AC_ARG_WITH(xslt-config, [  --with-xslt-config=PATH   Location of libxslt xslt-config []], xslt_config="$withval", xslt_config="")

if test "X$xslt_config" != "Xno" ; then
  if test "X$xslt_config" != "X" ; then
    AC_MSG_CHECKING(for $xslt_config)

    if test -x $xslt_config ; then
      XSLT_CONFIG=$xslt_config
      AC_MSG_RESULT(yes)
    else
      AC_MSG_ERROR([xslt-config not found at specified path $xslt_config])
    fi
  fi
  if test "X$XSLT_CONFIG" = "X" ; then
    AC_CHECK_PROGS(XSLT_CONFIG, xslt-config)
  fi
fi

dnl curl
AC_ARG_WITH(curl-config, [  --with-curl-config=PATH   Location of libcurl curl-config []], curl_config="$withval", curl_config="")

if test "X$curl_config" != "Xno" ; then
  if test "X$curl_config" != "X" ; then
    AC_MSG_CHECKING(for $curl_config)

    if test -f $curl_config ; then
      CURL_CONFIG=$curl_config
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no - searching PATH)
    fi
  fi
  if test "X$CURL_CONFIG" = "X" ; then
    AC_CHECK_PROGS(CURL_CONFIG, curl-config)
  fi
fi

libcurl_source=no
if test "X$CURL_CONFIG" != "X"; then
  LIBCURL_CFLAGS=`$CURL_CONFIG --cflags`
  LIBCURL_LIBS=`$CURL_CONFIG --libs`

  CPPFLAGS="$LIBCURL_CFLAGS $CPPFLAGS"
  LIBS="$LIBS $LIBCURL_LIBS"
  AC_CHECK_HEADER(curl/curl.h)
  AC_CHECK_FUNC(curl_easy_init, have_curl_easy_init=yes, have_curl_easy_init=no)

  AC_MSG_CHECKING(for libcurl via curl-config)
  if test $have_curl_easy_init = yes; then
    libcurl_source="curl-config"
    LIBCURL_VERSION=`$CURL_CONFIG --version | sed -e 's/^libcurl *//'`
  fi
  CPPFLAGS="$oCPPFLAGS"
  LIBS="$oLIBS"
else
  PKG_CHECK_MODULES([LIBCURL],[libcurl],[
    LIBCURL_VERSION=`$PKG_CONFIG libcurl --modversion`
    libcurl_source="pkg-config"
  ], [:])
  AC_MSG_CHECKING(for libcurl via pkg-config)
fi

if test "$libcurl_source" = "no"; then
  AC_MSG_RESULT(no - not found)
else
  AC_MSG_RESULT(yes - $LIBCURL_VERSION)
fi


AC_ARG_WITH(icu-config, [  --with-icu-config=PATH   Location of ICU icu-config []], icu_config="$withval", icu_config="")

have_icu=no
AC_MSG_CHECKING(for ICU)
if test "X$icu_config" != "Xno" -a "X$icu_config" != "X" ; then
  ICU_CONFIG=$icu_config
  ICU_VERSION=`$ICU_CONFIG --version`
  have_icu=yes
  AC_MSG_RESULT(yes - version $ICU_VERSION)
else
  AC_MSG_RESULT(no)
fi
dnl Note there is NO automated searching for icu-config


AC_ARG_WITH(www-config, [  --with-libwww-config=PATH Location of W3C libwww libwww-config []], libwww_config="$withval", libwww_config="")

if test "X$libwww_config" != "Xno" -a "X$libwww_config" != "X" ; then
  AC_MSG_WARN(libwww is no longer supported)
fi


have_libxml=0
need_libxml=0

oCPPFLAGS="$CPPFLAGS"
if test "X$XML_CONFIG" != X; then
  CPPFLAGS="$LIBXML_CFLAGS $CPPFLAGS"
  LIBS="$LIBS $LIBXML_LIBS"
  AC_CHECK_FUNC(xmlCreatePushParserCtxt, have_xmlCreatePushParserCtxt=yes, have_xmlCreatePushParserCtxt=no)
  AC_MSG_CHECKING(libxml library)
  if test $have_xmlCreatePushParserCtxt = yes; then
    have_libxml=1
    libxml_version_dec=`echo $LIBXML_VERSION | $AWK -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};'`
    libxml_min_version_dec=`echo $libxml_min_version | $AWK -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};'`
    AC_MSG_RESULT(yes - version $LIBXML_VERSION)
    if test $libxml_version_dec -lt $libxml_min_version_dec; then
       AC_MSG_WARN(Using libxml $LIBXML_VERSION is unsupported - $libxml_min_version or newer required.)
       have_libxml=0
    fi
  else
    AC_MSG_RESULT(no)
  fi

  AC_CHECK_HEADERS(libxml/nanohttp.h)
  AC_CHECK_HEADERS(libxml/parser.h)
  AC_CHECK_HEADERS(libxml/hash.h libxml/SAX2.h,,,
  [#ifdef HAVE_LIBXML_PARSER_H
#include <libxml/parser.h>
#endif
])

  if test "$ac_cv_header_libxml_parser_h" = no -a "$ac_cv_header_gnome_xml_parser_h" = no; then
    AC_MSG_WARN(libxml library found but not headers - disabling)
    have_libxml=0
  else
    AC_MSG_CHECKING(if libxml xmlEntity has name_length field)
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_LIBXML_PARSER_H
#include <libxml/parser.h>
#endif
]], [[xmlEntity foo; foo.name_length=0]])],
                [AC_MSG_RESULT(yes)
		AC_DEFINE([RAPTOR_LIBXML_ENTITY_NAME_LENGTH], [1], [does libxml struct xmlEntity have a field name_length])],
		[AC_MSG_RESULT(no)])

    AC_MSG_CHECKING(if libxml xmlEntity has etype field)
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_LIBXML_PARSER_H
#include <libxml/parser.h>
#endif
]], [[xmlEntity foo; foo.etype=0]])],
                [AC_MSG_RESULT(yes)
		AC_DEFINE([RAPTOR_LIBXML_ENTITY_ETYPE], [1], [does libxml struct xmlEntity have a field etype])],
		[AC_MSG_RESULT(no)])

    AC_MSG_CHECKING(if libxml xmlSAXHandler has initialized field)
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_LIBXML_PARSER_H
#include <libxml/parser.h>
#endif
]], [[xmlSAXHandler foo; foo.initialized=0]])],
                [AC_MSG_RESULT(yes)
		AC_DEFINE([RAPTOR_LIBXML_XMLSAXHANDLER_INITIALIZED], [1], [does libxml xmlSAXHandler have initialized field])],
		[AC_MSG_RESULT(no)])

    AC_MSG_CHECKING(if libxml xmlSAXHandler has externalSubset field)
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_LIBXML_PARSER_H
#include <libxml/parser.h>
#endif
]], [[xmlSAXHandler foo; foo.externalSubset=NULL]])],
                [AC_MSG_RESULT(yes)
		AC_DEFINE([RAPTOR_LIBXML_XMLSAXHANDLER_EXTERNALSUBSET], [1], [does libxml xmlSAXHandler have externalSubset field])],
		[AC_MSG_RESULT(no)])

    AC_CHECK_FUNCS(xmlSAX2InternalSubset xmlCtxtUseOptions)

    AC_MSG_CHECKING(if libxml has parser option XML_PARSE_NONET)
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_LIBXML_PARSER_H
#include <libxml/parser.h>
#endif
]], [[xmlParserOption foo; foo = XML_PARSE_NONET]])],
                [AC_MSG_RESULT(yes)
		AC_DEFINE([RAPTOR_LIBXML_XML_PARSE_NONET], [1], [does libxml have XML_PARSE_NONET])],
		[AC_MSG_RESULT(no)])
    AC_CHECK_HEADERS(libxml/HTMLparser.h)
    AC_MSG_CHECKING(if libxml has parser option HTML_PARSE_NONET)
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#ifdef HAVE_LIBXML_HTMLPARSER_H
#include <libxml/HTMLparser.h>
#endif
]], [[htmlParserOption foo; foo = HTML_PARSE_NONET]])],
                [AC_MSG_RESULT(yes)
		AC_DEFINE([RAPTOR_LIBXML_HTML_PARSE_NONET], [1], [does libxml have HTML_PARSE_NONET])],
		[AC_MSG_RESULT(no)])
  fi

fi
CPPFLAGS="$oCPPFLAGS"
LIBS="$oLIBS"


have_libxslt=0

if test "X$XSLT_CONFIG" != X; then
  CPPFLAGS="`$XSLT_CONFIG --cflags` $CPPFLAGS"
  LIBS="$LIBS `$XSLT_CONFIG --libs`"
  AC_CHECK_FUNC(xsltSaveResultToString, have_xsltSaveResultToString=yes, have_xsltSaveResultToString=no)
  AC_MSG_CHECKING(for system libxslt library)
  if test $have_xsltSaveResultToString = yes; then
    have_libxslt=1
    LIBXSLT_VERSION=`$XSLT_CONFIG --version`
    libxslt_version_dec=`echo $LIBXSLT_VERSION | $AWK -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};'`
    libxslt_min_version_dec=`echo $libxslt_min_version | $AWK -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};'`
    AC_MSG_RESULT(yes - version $LIBXSLT_VERSION)
    if test $libxslt_version_dec -lt $libxslt_min_version_dec; then
       AC_MSG_WARN(Using libxslt $LIBXSLT_VERSION is unsupported - $libxslt_min_version or newer required.)
       have_libxslt=0
    fi
  else
    AC_MSG_RESULT(no)
  fi

  AC_CHECK_FUNC(xsltInit)

  AC_CHECK_HEADERS(libxslt/xslt.h)
  if test "$ac_cv_header_libxslt_xslt_h" = no ; then
    AC_MSG_WARN(libxslt library found but not headers - disabling)
    have_libxslt_lib=0
    have_libxslt=0
  fi
fi
CPPFLAGS="$oCPPFLAGS"
LIBS="$oLIBS"


dnl Check for JSON library

AC_ARG_WITH(yajl, [  --with-yajl=DIR  YAJL installation directory or 'no' to disable (default=auto)], yajl_prefix="$withval", yajl_prefix="none")

if test "x$yajl_prefix" != "xno" ; then
  AC_MSG_CHECKING(for yajl installation)

  if test "x$yajl_prefix" = "xyes" ; then
    yajl_prefix="none"
  fi

  # Nothing given - search
  if test "X$yajl_prefix" = "Xnone" ; then
    for dir in /opt/local /usr/local /usr; do
      if test -r $dir/include/yajl/yajl_parse.h; then
        yajl_prefix=$dir
        break
      fi
    done
  fi

  if test "X$yajl_prefix" = "Xnone" ; then
    AC_MSG_RESULT(not found. Get it from http://lloyd.github.com/yajl/ and use --with-yajl=DIR if necessary to configure the installation directory.)
  else
    AC_MSG_RESULT($yajl_prefix)

    if test "$yajl_prefix" = "/usr"; then
      yajl_prefix=
    else
      LDFLAGS="-L$yajl_prefix/lib $LDFLAGS"
      CPPFLAGS="-I$yajl_prefix/include $CPPFLAGS"
    fi

    AC_CHECK_LIB(yajl, yajl_parse, libyajl=1, libyajl=0)
    AC_CHECK_LIB(yajl, yajl_tree_parse, yajl_api_version=2, yajl_api_version=1)
    AC_CHECK_HEADERS(yajl/yajl_parse.h)

    AC_MSG_CHECKING(YAJL API version)
    AC_MSG_RESULT($yajl_api_version)
    if test $yajl_api_version = 2; then
      AC_DEFINE_UNQUOTED(HAVE_YAJL2, 1, [YAJL has API version 2])
    fi

  fi
fi
CPPFLAGS="$oCPPFLAGS"
LIBS="$oLIBS"


dnl RDF Parsers
rdfxml_parser=no
ntriples_parser=no
turtle_parser=no
trig_parser=no
rss_parser=no
grddl_parser=no
guess_parser=yes
rdfa_parser=no
json_parser=no
nquads_parser=no

rdf_parsers_available="rdfxml ntriples turtle trig guess rss-tag-soup rdfa nquads"
rdf_parsers_enabled=


grddl_parser_ok=no
AC_MSG_CHECKING(GRDDL parser requirements)
if test $have_libxml = 1 -a $have_libxslt = 1; then
  AC_MSG_RESULT(yes)
  grddl_parser_ok=yes
  rdf_parsers_available="$rdf_parsers_available grddl"
else
  AC_MSG_RESULT(no - libxml2 and libxslt are both not available)
fi

json_parser_ok=no
AC_MSG_CHECKING(JSON parser requirements)
if test "X$ac_cv_header_yajl_yajl_parse_h" = Xyes -a "X$ac_cv_lib_yajl_yajl_parse" = Xyes; then
  AC_MSG_RESULT(yes)
  json_parser_ok=yes
  rdf_parsers_available="$rdf_parsers_available json"
else
  AC_MSG_RESULT(no - libyajl is not available)
fi



# This is needed because autoheader can't work out which computed
# symbols must be pulled from acconfig.h into config.h.in
if test "x" = "y"; then
  AC_DEFINE(RAPTOR_PARSER_RDFXML, 1, [Building RDF/XML parser])
  AC_DEFINE(RAPTOR_PARSER_NTRIPLES, 1, [Building N-Triples parser])
  AC_DEFINE(RAPTOR_PARSER_TURTLE, 1, [Building Turtle parser])
  AC_DEFINE(RAPTOR_PARSER_TRIG, 1, [Building TRiG parser])
  AC_DEFINE(RAPTOR_PARSER_RSS, 1, [Building RSS Tag Soup parser])
  AC_DEFINE(RAPTOR_PARSER_GRDDL, 1, [Building GRDDL parser])
  AC_DEFINE(RAPTOR_PARSER_GUESS, 1, [Building guess parser])
  AC_DEFINE(RAPTOR_PARSER_RDFA, 1, [Building RDFA parser])
  AC_DEFINE(RAPTOR_PARSER_JSON, 1, [Building JSON parser])
  AC_DEFINE(RAPTOR_PARSER_NQUADS, 1, [Building N-Quads parser])
fi

AC_MSG_CHECKING(RDF parsers required)
AC_ARG_ENABLE(parsers, [  --enable-parsers=LIST      Use RDF parsers (default=all)], parsers="$enableval") 
if test "X$parsers" = Xall -o "X$parsers" = X; then
  parsers="$rdf_parsers_available"
  AC_MSG_RESULT(all)
elif test "X$parsers" = Xnone; then
  parsers=
  AC_MSG_RESULT(none)
else
  AC_MSG_RESULT($parsers)
fi

for parser in $parsers; do
  p=$parser
  if test $p = rss-tag-soup; then
    p=rss
  fi

  if test $p = grddl; then
    if test $grddl_parser_ok != yes; then
      AC_MSG_WARN(GRDDL parser is not available)
      continue
    fi
  fi

  if test $p = json; then
    if test $json_parser_ok != yes; then
      AC_MSG_WARN(YAJL is not available)
      continue
    fi
  fi

  eval $p'_parser=yes'
  NAME=`echo $p | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`
  n=RAPTOR_PARSER_${NAME}
  AC_DEFINE_UNQUOTED($n)
  rdf_parsers_enabled="$rdf_parsers_enabled $parser"
done

use_nfc=no
if test $rdfxml_parser = yes; then
  need_libxml=1
  use_nfc=yes
fi

if test $rss_parser = yes; then
  need_libxml=1
fi

need_libxslt=0
if test $grddl_parser = yes; then
  need_libxml=1
  need_libxslt=1
fi

need_librdfa=no
if test $rdfa_parser = yes; then
  need_libxml=1
  need_librdfa=yes
fi

need_libyajl=0
if test $json_parser = yes; then
  need_libyajl=1
fi

AM_CONDITIONAL(RAPTOR_PARSER_RDFXML, test $rdfxml_parser = yes)
AM_CONDITIONAL(RAPTOR_PARSER_NTRIPLES, test $ntriples_parser = yes)
AM_CONDITIONAL(RAPTOR_PARSER_TURTLE, test $turtle_parser = yes)
AM_CONDITIONAL(RAPTOR_PARSER_TRIG, test $trig_parser = yes)
AM_CONDITIONAL(RAPTOR_PARSER_RSS, test $rss_parser = yes)
AM_CONDITIONAL(RAPTOR_PARSER_GRDDL, test $grddl_parser = yes)
AM_CONDITIONAL(RAPTOR_PARSER_GUESS, test $guess_parser = yes)
AM_CONDITIONAL(RAPTOR_PARSER_RDFA, test $rdfa_parser = yes)
AM_CONDITIONAL(RAPTOR_PARSER_JSON, test $json_parser = yes)
AM_CONDITIONAL(RAPTOR_PARSER_NQUADS, test $nquads_parser = yes)

AM_CONDITIONAL(LIBRDFA, test $need_librdfa = yes)


dnl RDF Serializers
rdfxml_serializer=no
ntriples_serializer=no
rdfxml_abbrev_serializer=no
turtle_serializer=no
rss_1_0_serializer=no
atom_serializer=no
dot_serializer=no
html_serializer=no
json_serializer=no
nquads_serializer=no

rdf_serializers_available="rdfxml rdfxml-abbrev turtle ntriples rss-1.0 dot html json atom nquads"

# This is needed because autoheader can't work out which computed
# symbols must be pulled from acconfig.h into config.h.in
if test "x" = "y"; then
  AC_DEFINE(RAPTOR_SERIALIZER_RDFXML, 1, [Building RDF/XML serializer])
  AC_DEFINE(RAPTOR_SERIALIZER_NTRIPLES, 1, [Building N-Triples serializer])
  AC_DEFINE(RAPTOR_SERIALIZER_RDFXML_ABBREV, 1, [Building RDF/XML-abbreviated serializer])
  AC_DEFINE(RAPTOR_SERIALIZER_TURTLE, 1, [Building Turtle serializer])
  AC_DEFINE(RAPTOR_SERIALIZER_RSS_1_0, 1, [Building RSS 1.0 serializer])
  AC_DEFINE(RAPTOR_SERIALIZER_ATOM, 1, [Building Atom 1.0 serializer])
  AC_DEFINE(RAPTOR_SERIALIZER_DOT, 1, [Building GraphViz DOT serializer])
  AC_DEFINE(RAPTOR_SERIALIZER_HTML, 1, [Building HTML Table serializer])
  AC_DEFINE(RAPTOR_SERIALIZER_JSON, 1, [Building JSON serializer])
  AC_DEFINE(RAPTOR_SERIALIZER_NQUADS, 1, [Building N-Quads serializer])
fi

AC_MSG_CHECKING(RDF serializers required)
AC_ARG_ENABLE(serializers, [  --enable-serializers=LIST  Use RDF serializers (default=all)], serializers="$enableval") 
if test "X$serializers" = Xall -o "X$serializers" = X; then
  serializers="$rdf_serializers_available"
  AC_MSG_RESULT(all)
elif test "X$serializers" = Xnone; then
  serializers=
  AC_MSG_RESULT(none)
else
  AC_MSG_RESULT($serializers)
fi

for serializer in $serializers; do
  s=`echo $serializer | tr '.-' '__'`
  eval $s'_serializer=yes'
  NAME=`echo $s | tr 'abcdefghijklmnopqrstuvwxyz' 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'`
  n=RAPTOR_SERIALIZER_${NAME}
  AC_DEFINE_UNQUOTED($n)
  rdf_serializers_enabled="$rdf_serializers_enabled $serializer"
done

AM_CONDITIONAL(RAPTOR_SERIALIZER_RDFXML, test $rdfxml_serializer = yes)
AM_CONDITIONAL(RAPTOR_SERIALIZER_NTRIPLES, test $ntriples_serializer = yes)
AM_CONDITIONAL(RAPTOR_SERIALIZER_RDFXML_ABBREV, test $rdfxml_abbrev_serializer = yes)
AM_CONDITIONAL(RAPTOR_SERIALIZER_TURTLE, test $turtle_serializer = yes)
AM_CONDITIONAL(RAPTOR_SERIALIZER_RSS_1_0, test $rss_1_0_serializer = yes)
AM_CONDITIONAL(RAPTOR_SERIALIZER_ATOM, test $atom_serializer = yes)
AM_CONDITIONAL(RAPTOR_SERIALIZER_DOT, test $dot_serializer = yes)
AM_CONDITIONAL(RAPTOR_SERIALIZER_HTML, test $html_serializer = yes)
AM_CONDITIONAL(RAPTOR_SERIALIZER_JSON, test $json_serializer = yes)
AM_CONDITIONAL(RAPTOR_SERIALIZER_NQUADS, test $nquads_serializer = yes)

AM_CONDITIONAL(RAPTOR_RSS_COMMON, test $rss_1_0_serializer = yes -o $rss_parser = yes)

dnl Enable NFC code only if enabled and used (rdfxml)
need_icu=no
if test $use_nfc = yes -a $have_icu = yes; then
  need_icu=yes
  AC_DEFINE([RAPTOR_ICU_NFC], 1, [Use ICU for Unicode NFC check])
fi
AM_CONDITIONAL(RAPTOR_ICU_NFC, test $need_icu = yes)



AC_MSG_CHECKING(XML names version)
AC_ARG_WITH(xml-names, [  --with-xml-names=1.1|1.0  Select XML version name checking (default=1.0)], xml_names="$withval", xml_names="1.0")
if test $xml_names = 1.1; then
  AC_DEFINE(RAPTOR_XML_1_1, 1, [Check XML 1.1 Names])
fi
AC_MSG_RESULT($xml_names)


have_libcurl=0
have_libfetch=0
need_libcurl=0
need_libxml_www=0
need_libfetch=0

if test "X$CURL_CONFIG" != X; then
  CPPFLAGS="$CPPFLAGS $LIBCURL_CFLAGS"
  LIBS="$LIBS $LIBCURL_LIBS"
  AC_CHECK_HEADER(curl/curl.h)
  AC_CHECK_FUNC(curl_easy_init, have_curl_easy_init=yes, have_curl_easy_init=no)

  AC_MSG_CHECKING(for libcurl library)
  if test $have_curl_easy_init = yes -a "$ac_cv_header_curl_curl_h" = yes; then
    libcurl_min_version_dec=`echo $libcurl_min_version | $AWK -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};'`
    libcurl_version_dec=`echo $LIBCURL_VERSION | $AWK -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};'`
    if test $libcurl_version_dec -lt $libcurl_min_version_dec; then
      AC_MSG_WARN(Using libcurl $LIBCURL_VERSION is unsupported - $libcurl_min_version or newer required.)
      AC_MSG_RESULT(no - version $LIBCURL_VERSION is too old)
    else
      have_libcurl=1
      AC_MSG_RESULT(yes - version $LIBCURL_VERSION via $libcurl_source)
    fi
  else
    AC_MSG_RESULT(no)
  fi
fi
LIBS="$oLIBS"
CPPFLAGS="$oCPPFLAGS"


if test "X$ac_cv_header_curl_curl_h" = Xyes; then
  AC_DEFINE([HAVE_CURL_CURL_H], 1, [Have curl/curl.h])
fi

if test $ac_cv_header_fetch_h = yes; then
  AC_CHECK_LIB(fetch, fetchXGetURL, have_libfetch=1)
  LIBS="$oLIBS"
fi


AC_ARG_WITH(www, [  --with-www=NAME           Use WWW library - curl (default), xml, libfetch, none], www="$withval", www="curl") 

for www_name in $www curl xml libfetch none; do
  case $www_name in
    curl)
      if test $have_libcurl = 1; then
        need_libcurl=1
        AC_DEFINE([RAPTOR_WWW_LIBCURL], 1, [Have libcurl WWW library])
        break
      fi
      ;;

    xml)
      if test $have_libxml = 1; then
        need_libxml=1
        need_libxml_www=1
	AC_DEFINE([RAPTOR_WWW_LIBXML], 1, [Have libxml available as a WWW library])
        break
      fi
      ;;

    libfetch)
      if test $have_libfetch = 1; then
        need_libfetch=1
	AC_DEFINE([RAPTOR_WWW_LIBFETCH], 1, [Have libfetch WWW library])
        break
      fi
      ;;

    none|no)
      need_libcurl=0
      need_libxml_www=0
      AC_DEFINE([RAPTOR_WWW_NONE], 1, [No WWW library])
      break
      ;;

    *)
      AC_MSG_ERROR(No such WWW library $www_name)
      ;;
esac
done

AC_MSG_CHECKING(WWW libraries available)
www_libraries_available=
if test $have_libcurl = 1; then
  www_libraries_available="$www_libraries_available libcurl $LIBCURL_VERSION"
fi
if test $have_libxml = 1; then
  www_libraries_available="$www_libraries_available libxml $LIBXML_VERSION"
fi
if test $have_libfetch = 1; then
  www_libraries_available="$www_libraries_available libfetch"
fi
AC_MSG_RESULT($www_libraries_available)


AC_MSG_CHECKING(WWW library to use)
www_library=
RAPTOR_WWW_LIBRARY=none
if test $need_libcurl = 1; then
  www_library="libcurl $LIBCURL_VERSION via $libcurl_source"
  RAPTOR_WWW_LIBRARY=libcurl
elif test $need_libxml_www = 1; then
  www_library="libxml $LIBXML_VERSION via $libxml_source"
  RAPTOR_WWW_LIBRARY=libxml
elif test $need_libfetch = 1; then
  www_library="libfetch"
  RAPTOR_WWW_LIBRARY=libfetch
else
  www_library=none
fi
AC_MSG_RESULT($www_library)
if test "X$www_library" = Xnone; then
  AC_MSG_WARN([No WWW library in use - only file: URLs will work])
  AC_MSG_WARN([Install libcurl, libxml2 or BSD libfetch for WWW access])
fi

if test $need_libcurl = 1; then
  CPPFLAGS="$CPPFLAGS $LIBCURL_CFLAGS"
  RAPTOR_LDFLAGS="$RAPTOR_LDFLAGS $LIBCURL_LIBS"

  if test libcurl_source = "pkg-config"; then
    if test "X$PKG_CONFIG_REQUIRES" != X; then
      PKG_CONFIG_REQUIRES="$PKG_CONFIG_REQUIRES,"
    fi
    PKG_CONFIG_REQUIRES="$PKG_CONFIG_REQUIRES libcurl >= $libcurl_min_version"
  fi

  AC_LIBOBJ(raptor_www_curl)
fi


dnl ICU for NFC check
AC_MSG_CHECKING(NFC library to use)
nfc_library=none
if test $need_icu = yes; then
  CPPFLAGS="$CPPFLAGS `$ICU_CONFIG --cppflags-searchpath`"
  RAPTOR_LDFLAGS="$RAPTOR_LDFLAGS `$ICU_CONFIG --ldflags-searchpath` -licuuc"
  AC_LIBOBJ(raptor_nfc_icu)
  nfc_library="ICU $ICU_VERSION"
fi
AC_MSG_RESULT($nfc_library)


have_lininn=no
have_inn_parsedate=no

oCPPFLAGS="$CPPFLAGS"
if test -d /usr/include/inn; then
  CPPFLAGS="$CPPFLAGS -I/usr/include/inn"
fi
AC_CHECK_HEADER(libinn.h)
CPPFLAGS="$oCPPFLAGS"

oCPPFLAGS="$CPPFLAGS"
if test $ac_cv_header_libinn_h = yes; then
  CPPFLAGS="$CPPFLAGS -I/usr/include/inn"
  LIBS="$LIBS -L/usr/lib/news -linn"
  AC_CHECK_LIB(inn, HashMessageID, have_libinn=yes)

  AC_MSG_CHECKING(parsedate in libinn)
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#include <libinn.h>
]], [[ parsedate("Sun Jun 12 00:04:09 BST 2005", NULL) ]])],
    [AC_DEFINE([HAVE_INN_PARSEDATE], [1], [INN parsedate function present])
     have_inn_parsedate=yes
     AC_MSG_RESULT(yes)],
    [AC_MSG_RESULT(no)])
fi
CPPFLAGS="$oCPPFLAGS"
LIBS="$oLIBS"


AC_MSG_CHECKING(date parsing source)
raptor_parsedate_needed=no
if test $have_inn_parsedate = yes; then
  CPPFLAGS="$CPPFLAGS -I/usr/include/inn"
  RAPTOR_LDFLAGS="$RAPTOR_LDFLAGS -L/usr/lib/news -linn"
  AC_MSG_RESULT(INN parsedate)
else
  if test $need_libcurl = 1; then
    AC_MSG_RESULT(libcurl curl_getdate)
  else
    raptor_parsedate_needed=yes
    AC_MSG_RESULT(raptor parsedate)
  fi
fi

AM_CONDITIONAL(PARSEDATE, test $raptor_parsedate_needed = yes)
if test $raptor_parsedate_needed = yes; then
  AC_DEFINE([HAVE_RAPTOR_PARSE_DATE], 1, [Raptor raptor_parse_date available])
fi


if test $need_libfetch = 1; then
  RAPTOR_LDFLAGS="$RAPTOR_LDFLAGS -lfetch"
  AC_LIBOBJ(raptor_www_libfetch)
fi


RAPTOR_XML_PARSER=none
if test $need_libxml = 1; then
  if test $have_libxml = 1; then
    AC_DEFINE(RAPTOR_XML_LIBXML, 1, [Use libxml XML parser])
  else
    AC_MSG_ERROR(libxml2 is not available - please get it from http://xmlsoft.org/)
  fi
  if test $need_libxml_www = 1; then
    AC_LIBOBJ(raptor_www_libxml)
  fi
  RAPTOR_LDFLAGS="$RAPTOR_LDFLAGS $LIBXML_LIBS"
  CPPFLAGS="$LIBXML_CFLAGS $CPPFLAGS"

  if test libxml_source = "pkg-config"; then
    if test "X$PKG_CONFIG_REQUIRES" != X; then
      PKG_CONFIG_REQUIRES="$PKG_CONFIG_REQUIRES,"
    fi
    PKG_CONFIG_REQUIRES="$PKG_CONFIG_REQUIRES libxml-2.0 >= $libxml_min_version"
  fi
  RAPTOR_XML_PARSER=libxml
fi
AM_CONDITIONAL(RAPTOR_XML_LIBXML, test $need_libxml = 1)


if test $need_libxslt = 1; then
  RAPTOR_LDFLAGS="$RAPTOR_LDFLAGS `$XSLT_CONFIG --libs`"
  CPPFLAGS="`$XSLT_CONFIG --cflags` $CPPFLAGS"
fi

if test $need_libyajl = 1; then
  if test "x$yajl_prefix" != "x"; then
    CPPFLAGS="-I$yajl_prefix/include $CPPFLAGS"
    RAPTOR_LDFLAGS="$RAPTOR_LDFLAGS -L$yajl_prefix/lib"
  fi
  RAPTOR_LDFLAGS="$RAPTOR_LDFLAGS -lyajl"
fi

RAPTOR_LIBTOOLLIBS=libraptor2.la
AC_SUBST(RAPTOR_LIBTOOLLIBS)


xml_parser="none needed"
if test $need_libxml = 1; then
  xml_parser="libxml $LIBXML_VERSION via $libxml_source"
fi


# Restore LIBS
LIBS="$oLIBS"

# Make final changes to cflags
MEM=
MEM_LIBS=
CPPFLAGS="-DRAPTOR_INTERNAL=1 $CPPFLAGS"

AC_ARG_WITH(memory-signing, [  --with-memory-signing     Sign allocated memory (default=no)], use_memory_signing="$withval", use_memory_signing="no") 
AC_MSG_CHECKING(using memory signing)
AC_MSG_RESULT($use_memory_signing);
if test "$use_memory_signing" = yes; then
  MEM=-DRAPTOR_MEMORY_SIGN=1
  MEM_LIBS=
fi

debug_messages=no

AC_ARG_ENABLE(debug, [  --enable-debug          Enable debug messages (default no).  ], debug_messages=$enableval)	
if test "$debug_messages" = "yes"; then
  AC_DEFINE([RAPTOR_DEBUG], [1], [Define to 1 if debug messages are enabled.])
fi

if test "$USE_MAINTAINER_MODE" = yes; then
  AC_DEFINE([MAINTAINER_MODE], [1], [Define to 1 if maintainer mode is enabled.])
  CPPFLAGS="$MAINTAINER_CPPFLAGS $CPPFLAGS"
fi


AC_SUBST(RAPTOR_LDFLAGS)

AC_SUBST(PKG_CONFIG_REQUIRES)

AC_SUBST(MEM)
AC_SUBST(MEM_LIBS)

AC_SUBST(RECHO)
AC_SUBST(RECHO_N)
AC_SUBST(RECHO_C)

# Features
# lists
RAPTOR_PARSERS=$rdf_parsers_enabled
AC_SUBST(RAPTOR_PARSERS)
RAPTOR_SERIALIZERS=$rdf_serializers_enabled
AC_SUBST(RAPTOR_SERIALIZERS)
# single values or none
AC_SUBST(RAPTOR_WWW_LIBRARY)
AC_SUBST(RAPTOR_XML_PARSER)

abs_top_srcdir=`cd $srcdir; pwd`
AC_SUBST(abs_top_srcdir)
abs_top_builddir=`pwd`
AC_SUBST(abs_top_builddir)

dnl automake 1.11
AM_SILENT_RULES([no])

AC_CONFIG_FILES([Makefile
raptor2.spec
raptor2.rdf
data/Makefile
docs/Makefile
docs/version.xml
examples/Makefile
scripts/Makefile
src/raptor2.h
src/Makefile
tests/Makefile
tests/feeds/Makefile
tests/grddl/Makefile
tests/ntriples/Makefile
tests/ntriples-2013/Makefile
tests/nquads-2013/Makefile
tests/rdfa/Makefile
tests/rdfa11/Makefile
tests/json/Makefile
tests/rdfxml/Makefile
tests/turtle/Makefile
tests/turtle-2013/Makefile
tests/trig/Makefile
utils/Makefile
librdfa/Makefile
raptor2.pc])

dnl Check for gtk-doc and docbook
GTK_DOC_CHECK([1.3])

AC_OUTPUT

AC_MSG_RESULT([
Raptor build summary:
  RDF parsers available     : $rdf_parsers_available
  RDF parsers enabled       :$rdf_parsers_enabled
  RDF serializers available : $rdf_serializers_available
  RDF serializers enabled   :$rdf_serializers_enabled
  XML parser                : $xml_parser
  WWW library               : $www_library
  NFC check library         : $nfc_library
])

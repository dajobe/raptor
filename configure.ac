dnl -*- Mode: sh -*-
dnl
dnl configure.in - autoconf file for Raptor
dnl (Process this file with autoconf to produce a configure script.)
dnl
dnl $Id$
dnl
dnl Copyright (C) 2000-2003 David Beckett - http://purl.org/net/dajobe/
dnl Institute for Learning and Research Technology - http://www.ilrt.org/
dnl University of Bristol - http://www.bristol.ac.uk/
dnl 
dnl This package is Free Software or Open Source available under the
dnl following licenses (these are alternatives):
dnl   1. GNU Lesser General Public License (LGPL)
dnl   2. GNU General Public License (GPL)
dnl   3. Mozilla Public License (MPL)
dnl 
dnl See LICENSE.html or LICENSE.txt at the top of this package for the
dnl full license terms.
dnl 


AC_INIT(Raptor RDF Parser, 0.9.9, redland-dev@lists.librdf.org, raptor)
AC_PREREQ(2.50)
AC_CONFIG_SRCDIR(raptor_parse.c)
AC_REVISION($Revision$)

libxml_min_version=2.4.0

AM_INIT_AUTOMAKE(1.6)
AM_CONFIG_HEADER(config.h)
AM_MAINTAINER_MODE

release_version=no
AC_ARG_ENABLE(release, [  --enable-release        Turn on optimizations.  ], \
 if test "$enableval" = "yes"; then \
	release_version=yes
fi;)	

if test $release_version = no; then 
  CFLAGS=`echo $CFLAGS | sed s/-O2//`
  CXXFLAGS=`echo $CXXFLAGS | sed s/-O2//`
  CPPFLAGS=`echo $CPPFLAGS | sed s/-O2//`
fi

dnl Checks for programs.
AC_CANONICAL_HOST
AM_SANITY_CHECK
AM_PROG_CC_STDC
AM_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Find a tar command for 'make dist'
AC_CHECK_PROGS(TAR, gnutar gtar tar)

AM_MISSING_PROG(ACLOCAL, aclocal, $missing_dir)
AM_MISSING_PROG(AUTOCONF, autoconf, $missing_dir)
AM_MISSING_PROG(AUTOMAKE, automake, $missing_dir)
AM_MISSING_PROG(AUTOHEADER, autoheader, $missing_dir)



dnl compiler checks
# if using gcc...
if test "$ac_cv_prog_gcc" = yes; then
  STANDARD_CFLAGS=
  MAINTAINER_CFLAGS="-Wall -Wshadow -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations -Wredundant-decls"
else
  STANDARD_CFLAGS=
  MAINTAINER_CFLAGS=
fi



dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(errno.h stdlib.h unistd.h string.h limits.h dmalloc.h getopt.h expat.h xmlparse.h sys/stat.h curl/curl.h libxml/nanohttp.h)


dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_BIGENDIAN

dnl need to change quotes to allow square brackets
changequote(<<, >>)dnl
version_major=`echo $VERSION | sed -e 's/^\([^\.]*\)\.\([^\.]*\)\.\(.*\)$/\1/'`
version_minor=`echo $VERSION | sed -e 's/^\([^\.]*\)\.\([^\.]*\)\.\(.*\)$/\2/'`
version_release=`echo $VERSION | sed -e 's/^\([^\.]*\)\.\([^\.]*\)\.\(.*\)$/\3/'`
changequote([, ])dnl

AC_DEFINE_UNQUOTED(RAPTOR_VERSION_MAJOR, $version_major, [Major version number])
AC_DEFINE_UNQUOTED(RAPTOR_VERSION_MINOR, $version_minor, [Minor version number])
AC_DEFINE_UNQUOTED(RAPTOR_VERSION_RELEASE, $version_release, [Release version number])

# syntax: CURRENT[:REVISION[:AGE]]
RAPTOR_LIBTOOL_VERSION=0:0:0
AC_SUBST(RAPTOR_LIBTOOL_VERSION)


dnl Checks for library functions.
AC_CHECK_FUNCS(getopt getopt_long stricmp strcasecmp vsnprintf)

# was AC_REPLACE_FUNCS(strcasecmp) but strcasecmp has two alternatives:
# the other non-standard function stricmp or a local alternative
if test $ac_cv_func_stricmp = no -a $ac_cv_func_stricmp = no; then
  AC_LIBOBJ([strcasecmp])
fi

if test $ac_cv_func_getopt = no -a $ac_cv_func_getopt_long = no; then
  RAPPER_EXTRA_OBJS=getopt.$(OBJEXT)
  AC_SUBST([RAPPER_EXTRA_OBJS])
fi


AC_MSG_CHECKING(whether need to declare optind)
AC_TRY_LINK([#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif], [int x=optind;],
            AC_MSG_RESULT(no),
            AC_DEFINE(NEED_OPTIND_DECLARATION, 1, [need 'extern int optind' declaration?])
            AC_MSG_RESULT(yes))



if test $ac_cv_func_vsnprintf = yes; then
  AC_MSG_CHECKING(vsnprintf has C99 compatible return value)
  AC_TRY_RUN([#include <stdarg.h>
int is_c99(char *s, ...) {
  char buffer[32];
  va_list args;
  int r;
  va_start(args, s);
  r = vsnprintf(buffer, 5, s, args);
  va_end(args);

  return (r == 7);
}

int main(int argc, char* argv) {
  return is_c99("1234567");
}], AC_MSG_RESULT(no),
    AC_DEFINE(HAVE_C99_VSNPRINTF, 1, [vsnprint has C99 compatible return value])
    AC_MSG_RESULT(yes))
fi


AC_MSG_CHECKING(for redland sources in parent directory)
if test -d $srcdir/../librdf -a -r $srcdir/../librdf/rdf_node.c ; then
  have_redland=1
  AC_MSG_RESULT(yes)
  AC_DEFINE(RAPTOR_IN_REDLAND, 1, [is this being built inside redland?])
  REDLAND_LIBS="../librdf/librdf.la"
  REDLAND_CFLAGS=" -I\$(srcdir)/../librdf"
  AC_SUBST(REDLAND_LIBS)
  AC_SUBST(REDLAND_CFLAGS)
else
  AC_MSG_RESULT(no)
fi


dnl Checks for XML parsers
have_expat=0
have_expat_lib=0
have_expat_source=0
need_expat=0
need_expat_source=0
expat_source_dir=
expat_obj_dir=
AC_MSG_CHECKING(for expat sources)
if test -d $srcdir/expat; then
  have_expat_source=1
  have_expat=1
  expat_source_dir="\$(top_srcdir)/expat"
  expat_obj_dir="\$(top_builddir)/expat"
  AC_MSG_RESULT(yes - expat)
elif test "X$have_redland" = X1 -a -d $srcdir/../expat; then
  have_expat_source=1
  have_expat=1
  expat_source_dir="\$(top_srcdir)/../expat"
  expat_obj_dir="\$(top_builddir)/../expat"
  AC_MSG_RESULT(yes - ../expat via redland)
else
  AC_MSG_RESULT(no)
fi

oLIBS="$LIBS"
AC_CHECK_LIB(xmltok, main, xmlt=1, xmlt=0)

# Raptor needs XML_ParserCreate expat function (not namespace one)
AC_CHECK_LIB(xmlparse, XML_ParserCreate, xmlp=1, xmlp=0, -lxmltok)

LIBS="$oLIBS"
AC_CHECK_LIB(expat, XML_ParserCreate, libexpat=1, libexpat=0)

LIBS="$oLIBS"
AC_MSG_CHECKING(for working expat in libxmlparse and libxmltok)
if test $xmlp = 1 -a $xmlt = 1 -a  $ac_cv_header_xmlparse_h = yes; then
  LIBS="$LIBS -lxmlparse -lxmltok"
  AC_TRY_RUN([#include <stdio.h>
	     main() {XML_ParserCreate(NULL); return(0);}],
             worked=yes, worked=no, worked=no)
  if test $worked = yes; then
    # Old expat
    have_expat_lib=1
    have_expat=1
    expat_libs="-lxmlparse -lxmltok"

    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
  fi
else
  AC_MSG_RESULT(no)
fi

LIBS="$oLIBS"

AC_MSG_CHECKING(for working expat in libexpat)
if test $libexpat = 1 -a $ac_cv_header_expat_h = yes ; then
  LIBS="$LIBS -lexpat"
  AC_TRY_RUN([#include <stdio.h>
             main() {XML_ParserCreate(NULL); return(0);}],
             worked=yes, worked=no, worked=no)
  if test $worked = yes; then
    # New expat - expat-1.95.0 or later
    have_expat_lib=1
    have_expat=1
    expat_libs="-lexpat"

    AC_MSG_RESULT(yes)

    AC_MSG_CHECKING(for expat support of UTF-8 BOM)
    AC_TRY_RUN([
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#ifdef HAVE_EXPAT_H
#include <expat.h>
#else
#ifdef HAVE_XMLPARSE_H
#include <xmlparse.h>
#endif
#endif

int main(int argc, char **argv) {
  const char *xml_buffer=
     /* UTF-8 BOM */
     "\xef\xbb\xbf<?xml version='1.0' encoding='UTF-8'?><tag/>";
  XML_Parser xp=XML_ParserCreate(NULL);
  int  len=strlen(xml_buffer);
  
  /* This might cause an error on expat 1.95.1 */
  int rc=XML_Parse(xp, xml_buffer, len, 1);

  /* if expat gives an error ... */
  if(!rc) {
    /* then the next line will crash in normal_updatePosition  */
    int line=XML_GetCurrentLineNumber(xp);
  }
  XML_ParserFree(xp);

  return 0;
}
],
            AC_MSG_RESULT(yes),
            AC_DEFINE(EXPAT_UTF8_BOM_CRASH, 1, [does expat crash when it sees an initial UTF8 BOM?])
            AC_MSG_RESULT(no)
            AC_MSG_WARN(The available expat crashes on XML UTF-8 BOM in documents)
            AC_MSG_WARN(Fix either by using libxml or expat 1.95.2+))

  else
    AC_MSG_RESULT(no)
  fi
else
  AC_MSG_RESULT(no)
fi

LIBS="$oLIBS"

AC_ARG_WITH(xml2-config, [  --with-xml2-config=PATH Location of libxml xml2-config []], xml2_config="$withval", xml2_config="")

if test "X$xml2_config" != "X" ; then
  AC_MSG_CHECKING(for $xml2_config)

  if test -f $xml2_config ; then
    XML_CONFIG=$xml2_config
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no - searching PATH)
  fi
fi
if test "X$XML_CONFIG" = "X"; then
  AC_CHECK_PROGS(XML_CONFIG, xml2-config)
fi

AC_ARG_WITH(curl-config, [  --with-curl-config=PATH Location of libcurl curl-config []], curl_config="$withval", curl_config="")

if test "X$curl_config" != "X" ; then
  AC_MSG_CHECKING(for $curl_config)

  if test -f $curl_config ; then
    CURL_CONFIG=$curl_config
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no - searching PATH)
  fi
fi
if test "X$CURL_CONFIG" = "X"; then
  AC_CHECK_PROGS(CURL_CONFIG, curl-config)
fi


AC_ARG_WITH(pkg-config, [  --with-pkg-config=PATH Location of GNOME pkg-config []], pkg_config="$withval", pkg_config="")

if test "X$pkg_config" != "X" ; then
  AC_MSG_CHECKING(for $pkg_config)

  if test -f $pkg_config ; then
    PKG_CONFIG=$pkg_config
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no - searching PATH)
  fi
fi
if test "X$PKG_CONFIG" = "X"; then
  AC_CHECK_PROGS(PKG_CONFIG, pkg-config)
fi


AC_ARG_WITH(www-config, [  --with-libwww-config=PATH Location of W3C libwww libwww-config []], libwww_config="$withval", libwww_config="")

if test "X$libwww_config" != "X" ; then
  AC_MSG_CHECKING(for $libwww_config)

  if test -f $libwww_config ; then
    LIBWWW_CONFIG=$libwww_config
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no - searching PATH)
  fi
fi
if test "X$LIBWWW_CONFIG" = "X"; then
  AC_CHECK_PROGS(LIBWWW_CONFIG, libwww-config)
fi


have_libxml=0
have_libxml_lib=0
have_libxml_source=0
need_libxml=0
need_libxml_source=0
AC_MSG_CHECKING(for libxml sources)
if test -d $srcdir/libxml -a -r $srcdir/libxml/libxml.spec ; then
  have_libxml_source=1
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi

oCPPFLAGS="$CPPFLAGS"
oLIBS="$LIBS"
if test "X$XML_CONFIG" != X; then
  LIBS="$LIBS `$XML_CONFIG --libs`"
  AC_CHECK_FUNC(xmlCreatePushParserCtxt, have_xmlCreatePushParserCtxt=yes, have_xmlCreatePushParserCtxt=no)
  AC_MSG_CHECKING(for system (GNOME) libxml library)
  if test $have_xmlCreatePushParserCtxt = yes; then
    have_libxml_lib=1
    have_libxml=1
    CPPFLAGS="`$XML_CONFIG --cflags` $CPPFLAGS"
    LIBXML_VERSION=`$XML_CONFIG --version`
    libxml_version_dec=`echo $LIBXML_VERSION | awk -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};'`
    libxml_min_version_dec=`echo $libxml_min_version | awk -F. '{printf("%d\n", 10000*$1 + 100*$2 + $3)};'`
    AC_MSG_RESULT(yes - version $LIBXML_VERSION)
    if test $libxml_version_dec -lt $libxml_min_version_dec; then
       AC_MSG_WARN(Using libxml $LIBXML_VERSION is unsupported - $libxml_min_version or newer required.)
       have_libxml_lib=0
       have_libxml=0
    fi
  else
    AC_MSG_RESULT(no)
  fi

  AC_CHECK_HEADERS(libxml/parser.h libxml/hash.h)

  if test "$ac_cv_header_libxml_parser.h" = no -a "$ac_cv_header_gnome_xml_parser.h" = no; then
    AC_MSG_WARN(libxml library found but not headers - disabling)
    have_libxml_lib=0
    have_libxml=0
  else
    AC_MSG_CHECKING(if libxml xmlEntity has name_length field)
    AC_TRY_LINK([
#ifdef HAVE_LIBXML_PARSER_H
#include <libxml/parser.h>
#endif
], [xmlEntity foo; foo.name_length=0],
                AC_MSG_RESULT(yes)
		AC_DEFINE(RAPTOR_LIBXML_ENTITY_NAME_LENGTH,1,[does libxml struct xmlEntity have a field name_length]),
		AC_MSG_RESULT(no))

    AC_MSG_CHECKING(if libxml xmlEntity has etype field)
    AC_TRY_LINK([
#ifdef HAVE_LIBXML_PARSER_H
#include <libxml/parser.h>
#endif
], [xmlEntity foo; foo.etype=0],
                AC_MSG_RESULT(yes)
		AC_DEFINE(RAPTOR_LIBXML_ENTITY_ETYPE, 1, [does libxml struct xmlEntity have a field etype]),
		AC_MSG_RESULT(no))

    AC_MSG_CHECKING(if libxml xmlSAXHandler has initialized field)
    AC_TRY_LINK([
#ifdef HAVE_LIBXML_PARSER_H
#include <libxml/parser.h>
#endif
], [xmlSAXHandler foo; foo.initialized=0],
                AC_MSG_RESULT(yes)
		AC_DEFINE(RAPTOR_LIBXML_XMLSAXHANDLER_INITIALIZED, 1, [does libxml xmlSAXHandler have initialized field]),
		AC_MSG_RESULT(no))

    AC_MSG_CHECKING(if libxml xmlSAXHandler has externalSubset field)
    AC_TRY_LINK([
#ifdef HAVE_LIBXML_PARSER_H
#include <libxml/parser.h>
#endif
], [xmlSAXHandler foo; foo.externalSubset=NULL],
                AC_MSG_RESULT(yes)
		AC_DEFINE(RAPTOR_LIBXML_XMLSAXHANDLER_EXTERNALSUBSET, 1, [does libxml xmlSAXHandler have externalSubset field]),
		AC_MSG_RESULT(no))

    AC_CHECK_FUNC(xmlUseNewParser, AC_DEFINE(RAPTOR_LIBXML_XMLUSENEWPARSER, 1, [does libxml have xmlUseNewParser]))
  fi

fi
CPPFLAGS="$oCPPFLAGS"
LIBS="$oLIBS"


AC_ARG_WITH(xml-parser, [  --with-xml-parser=NAME  Use XML parser - libxml (default), expat], xml_parser="$withval", xml_parser="libxml") 

for xml_parser_name in $xml_parser libxml expat; do
  case $xml_parser_name in
    expat)
      if test $have_expat_lib = 1; then
	need_expat=1
	AC_DEFINE(RAPTOR_XML_EXPAT, 1, [Use expat XML parser])
        break
      elif test $have_expat_source = 1; then
	need_expat=1
	need_expat_source=1
	AC_DEFINE(RAPTOR_XML_EXPAT, 1, [Use expat XML parser])
        break
      fi
      ;;

    libxml)
      if test $have_libxml_lib = 1; then
	need_libxml=1
	AC_DEFINE(RAPTOR_XML_LIBXML, 1, [Use libxml XML parser])
        break
      elif test $have_libxml_source = 1; then
	need_libxml=1
	need_libxml_source=1
	AC_DEFINE(RAPTOR_XML_LIBXML, 1, [Use libxml XML parser])
        break
      fi
      ;;

    *)
      AC_MSG_ERROR(No such XML parser $xml_parser_name)
      ;;
esac
done


AC_MSG_CHECKING(XML parser to use)
result=
if test $need_libxml = 1; then
  if test $need_libxml_source = 1; then
    result="$result libxml(source)"
  else
    result="$result libxml(system)"
  fi
elif test $need_expat = 1; then
  if test $need_expat_source = 1; then
    result="$result expat(source)"
  else
    result="$result expat(system)"
  fi
else
  AC_MSG_ERROR(No XML parser available - please install expat or libxml)
fi
AC_MSG_RESULT($result)


have_libcurl=0
have_libwww=0
need_libcurl=0
need_libxml_www=0
need_libwww=0

oCPPFLAGS="$CPPFLAGS"
if test "X$CURL_CONFIG" != X; then
  oLIBS="$LIBS"
  LIBS="$LIBS `$CURL_CONFIG --libs`"
  AC_CHECK_FUNC(curl_easy_init, have_curl_easy_init=yes, have_curl_easy_init=no)
  LIBS="$oLIBS"
  AC_MSG_CHECKING(for libcurl library)
  if test $have_curl_easy_init = yes; then
    have_libcurl=1
    LIBCURL_VERSION=`$CURL_CONFIG --version | sed -e 's/^libcurl *//'`
    AC_MSG_RESULT(yes - version $LIBCURL_VERSION)
  else
    AC_MSG_RESULT(no)
  fi
fi


if test "X$LIBWWW_CONFIG" != X; then
  oLIBS="$LIBS"
  LIBS="$LIBS `$LIBWWW_CONFIG --libs`"
  AC_CHECK_FUNC(HTLibInit, have_libwww_htlibinit=yes, have_libwww_htlibinit=no)
  LIBS="$oLIBS"
  AC_MSG_CHECKING(for W3C libwww library)
  if test $have_libwww_htlibinit = yes; then
    have_libwww=1
    LIBWWW_VERSION=`$LIBWWW_CONFIG --version`
    AC_MSG_RESULT(yes - version $LIBWWW_VERSION)
  else
    AC_MSG_RESULT(no)
  fi
fi


AC_ARG_WITH(www, [  --with-www=NAME         Use WWW library - curl (default), xml, libwww, none], www="$withval", www="curl") 

for www_name in $www curl xml libwww none; do
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

    libwww)
      if test $have_libwww = 1; then
        need_libwww=1
	AC_DEFINE([RAPTOR_WWW_LIBWWW], 1, [Have W3C libwww WWW library])
        break
      fi
      ;;

    none)
      need_libcurl=0
      need_libxml_www=0
      need_libwww=0
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
  www_libraries_available="$www_libraries_available $LIBCURL_VERSION"
fi
if test $have_libxml = 1; then
  if test $need_libxml_source = 1; then
    www_libraries_available="$www_libraries_available libxml(source)"
  else
    www_libraries_available="$www_libraries_available libxml(system $LIBXML_VERSION)"
  fi
fi
if test $have_libwww = 1; then
  www_libraries_available="$www_libraries_available libwww $LIBWWW_VERSION"
fi
AC_MSG_RESULT($www_libraries_available)


AC_MSG_CHECKING(WWW library to use)
www_library=
if test $need_libcurl = 1; then
  www_library="libcurl $LIBCURL_VERSION"
elif test $need_libxml_www = 1; then
  if test $need_libxml_source = 1; then
    www_library="libxml(source)"
  else
    www_library="libxml(system $LIBXML_VERSION)"
  fi
elif test $need_libwww = 1; then
  www_library="libwww $LIBWWW_VERSION"
else
  www_library=none
fi
AC_MSG_RESULT($www_library)
if test "X$www_library" = Xnone; then
  AC_MSG_WARN([No WWW library in use - only file: URLs will work])
  AC_MSG_WARN([Install libcurl, libxml2, W3C libwww to provide WWW access])
fi


if test $need_libcurl = 1; then
  CPPFLAGS="$CPPFLAGS `$CURL_CONFIG --cflags`"
  LIBS="$LIBS `$CURL_CONFIG --libs`"
  AC_LIBOBJ(raptor_www_curl)
fi

if test $need_libwww = 1; then
  CPPFLAGS="$CPPFLAGS `$LIBWWW_CONFIG --cflags`"
  LIBS="$LIBS `$LIBWWW_CONFIG --libs`"
  AC_LIBOBJ(raptor_www_libwww)
fi


if test $need_libxml = 1; then

  if test $need_libxml_www = 1; then
    AC_LIBOBJ(raptor_www_libxml)
  fi
  if test $need_libxml_source = 1; then
    SD="$SD libxml"
    (cd libxml && ./configure --cache=../config.cache --enable-shared=no)
    CPPFLAGS="-I$srcdir/libxml $CPPFLAGS"
    LIBS="$LIBS -Llibxml -llibxml"
  else
    LIBS="$LIBS `$XML_CONFIG --libs`"
    CPPFLAGS="`$XML_CONFIG --cflags` $CPPFLAGS"
  fi
fi


if test $need_expat = 1; then
  # Only build local copy if it is needed
  if test $need_expat_source = 1; then
    SD="$SD expat"
    CPPFLAGS="-I$expat_source_dir/xmlparse $CPPFLAGS"
    LIBS="$LIBS $expat_obj_dir/xmlparse/xmlparse.o $expat_obj_dir/xmlparse/hashtable.o $expat_obj_dir/xmltok/xmlrole.o $expat_obj_dir/xmltok/xmltok.o"
  else
    LIBS="$LIBS $expat_libs"
  fi
fi


xml_parsers_available=
if test $need_libxml = 1; then
  if test $need_libxml_source = 1; then
    xml_parsers_available="$xml_parsers_available libxml(source)"
  else
    xml_parsers_available="$xml_parsers_available libxml(system $LIBXML_VERSION)"
  fi
fi
if test $need_expat = 1; then
  if test $need_expat_source = 1; then
    xml_parsers_available="$xml_parsers_available expat(source)"
  else
    if test $libexpat = 1; then
      xml_parsers_available="$xml_parsers_available expat(system libexpat)"
    else
      xml_parsers_available="$xml_parsers_available expat(system libxmlparse,libxmltok)"
    fi
  fi
fi


# Make final changes to cflags
MEM=
MEM_LIBS=
CPPFLAGS="-DRAPTOR_INTERNAL=1 $CPPFLAGS"
if test "$USE_MAINTAINER_MODE" = yes; then
  if test "$ac_cv_header_dmalloc_h" = yes; then
    MEM=-DRAPTOR_MEMORY_DEBUG_DMALLOC=1
    MEM_LIBS=-ldmalloc
  fi
  CPPFLAGS="-g -DRAPTOR_DEBUG=1 $CPPFLAGS"
fi
STANDARD_CFLAGS="$STANDARD_CFLAGS $CFLAGS"
if test "$USE_MAINTAINER_MODE" = yes; then
  CFLAGS="$MAINTAINER_CFLAGS $CFLAGS"
fi

AC_SUBST(MEM)
AC_SUBST(MEM_LIBS)
AC_SUBST(STANDARD_CFLAGS)

ECHO_N="$ECHO_N"
ECHO_C="$ECHO_C"
AC_SUBST(ECHO_N)
AC_SUBST(ECHO_C)

abs_top_srcdir=`cd $srcdir; pwd`
AC_SUBST(abs_top_srcdir)
abs_top_builddir=`pwd`
AC_SUBST(abs_top_builddir)

AC_OUTPUT(Makefile
raptor.spec
raptor-config
raptor-src-config
tests/Makefile
win32/Makefile
debian/Makefile,
[chmod +x raptor-config raptor-src-config])


AC_MSG_RESULT([
Raptor build summary:
  XML parser :$xml_parsers_available
  WWW library: $www_library
])

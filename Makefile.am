# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor
#
# $Id$
#
# Copyright (C) 2000-2001 David Beckett - http://purl.org/net/dajobe/
# Institute for Learning and Research Technology - http://www.ilrt.org/
# University of Bristol - http://www.bristol.ac.uk/
# 
# This package is Free Software or Open Source available under the
# following licenses (these are alternatives):
#   1. GNU Lesser General Public License (LGPL)
#   2. GNU General Public License (GPL)
#   3. Mozilla Public License (MPL)
# 
# See LICENSE.html or LICENSE.txt at the top of this package for the
# full license terms.
# 


bin_PROGRAMS = rapper
bin_SCRIPTS = raptor-config
noinst_SCRIPTS = raptor-src-config
lib_LTLIBRARIES = libraptor.la

include_HEADERS = raptor.h ntriples.h

noinst_HEADERS = raptor_internal.h win32_config.h

man_MANS = rapper.1 libraptor.3

rapper_SOURCES = rdfdump.c
rapper_LDADD = libraptor.la @REDLAND_LIBS@
rapper_DEPENDENCIES = @REDLAND_LIBS@
libraptor_la_SOURCES = raptor_parse.c ntriples_parse.c \
raptor_uri.c raptor_libxml.c raptor_win32.c raptor_locator.c \
raptor_namespace.c raptor_qname.c raptor_general.c raptor_utf8.c

libraptor_la_LDFLAGS = -version-info @RAPTOR_LIBTOOL_VERSION@
libraptor_la_LIBADD = @LTLIBOBJS@

EXTRA_DIST=ChangeLog \
README      NEWS      LICENSE.txt \
README.html NEWS.html LICENSE.html INSTALL.html \
MPL.html \
raptor-config.in raptor-src-config.in \
autogen.sh \
raptor.spec.in \
dc.rdf \
raptor_cc.gperf \
raptor_cc.c strcasecmp.c \
$(man_MANS)


SUBDIRS= debian tests win32

TESTS=raptor_cc_test raptor_uri_test raptor_namespace_test strcasecmp_test

CLEANFILES=$(TESTS)

# Use tar, whatever it is called (better be GNU tar though)
TAR=@TAR@

# Why is this not in the default makefile?
CC=@CC@

# Memory debugging alternatives
MEM=@MEM@
MEM_LIBS=@MEM_LIBS@

# 1) None (use standard functions directly)
#MEM=
#MEM_LIBS=

# 2) Use dmalloc library
#MEM=-DRAPTOR_MEMORY_DEBUG_DMALLOC=1
#MEM_LIBS=-ldmalloc


CPPFLAGS=@CPPFLAGS@ @REDLAND_CPPFLAGS@ $(MEM)
STANDARD_CFLAGS=@STANDARD_CFLAGS@ $(MEM)
LIBS=@LIBS@ $(MEM_LIBS)

LOCAL_LIB_DIR=lib

# Create some text files from HTML sources
LYNX=lynx
HTML_TO_TEXT=TERM=vt100 $(LYNX) -dump -nolist

SUFFIXES = .html .txt

.html.txt:
	$(HTML_TO_TEXT) $< > $@

README: README.html
	$(HTML_TO_TEXT) $< > $@

NEWS: NEWS.html
	$(HTML_TO_TEXT) $< > $@

raptor_cc.c : $(srcdir)/raptor_cc.gperf
	 gperf --struct-type --duplicates --language=ANSI-C --readonly-tables --delimiters=\; $< > $@


# Some people need a little help ;-)
test: check

raptor_cc_test: $(srcdir)/raptor_cc.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE -static $(srcdir)/raptor_cc.c libraptor.la $(LIBS)

raptor_uri_test: $(srcdir)/raptor_uri.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE -static $(srcdir)/raptor_uri.c libraptor.la $(LIBS)

raptor_namespace_test: $(srcdir)/raptor_namespace.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE -static $(srcdir)/raptor_namespace.c libraptor.la $(LIBS) @REDLAND_LIBS@

strcasecmp_test: $(srcdir)/strcasecmp.c
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE -static $(srcdir)/strcasecmp.c

deb: distcheck
	rm -rf $(distdir)
	tar xfz $(distdir).tar.gz
	cd $distdir && dpkg-buildpackage -rfakeroot

dist-hook: README NEWS
	@for file in README NEWS; do \
          if test -r $(srcdir)/$$file; then \
	    rm -f $(distdir)/$$file; \
	    cp -p $(srcdir)/$$file $(distdir)/$$file; \
	  fi; \
	done

@SET_MAKE@

../librdf/librdf.la:
	cd ../librdf && $(MAKE) librdf.la

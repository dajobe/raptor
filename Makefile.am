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


bin_PROGRAMS = rdfdump
lib_LTLIBRARIES = libraptor.la

include_HEADERS = raptor.h ntriples.h

# Create the test program, statically linked (easier to debug)
rdfdump_SOURCES = rdfdump.c
rdfdump_LDADD = libraptor.la
rdfdump_LDFLAGS = -static

libraptor_la_SOURCES = raptor_parse.c ntriples_parse.c \
raptor_uri.c

libraptor_la_LDFLAGS = -version-info @RAPTOR_LIBTOOL_VERSION@


EXTRA_DIST=ChangeLog \
README      NEWS      LICENSE.txt \
README.html NEWS.html LICENSE.html \
INSTALL.html MPL.html \
autogen.sh \
raptor.spec.in \
dc.rdf \
raptor_cc.gperf \
raptor_cc.c


SUBDIRS= \
	tests \
	win32

TESTS=raptor_cc_test raptor_uri_test

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


CPPFLAGS=@CPPFLAGS@ $(MEM)
STANDARD_CFLAGS=@STANDARD_CFLAGS@ $(MEM)
LIBS=@LIBS@ $(MEM_LIBS)


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

raptor_cc.c : raptor_cc.gperf
	 gperf --struct-type --duplicates --language=ANSI-C --readonly-tables --delimiters=\; $? > $@


# Some people need a little help ;-)
test: check

raptor_cc_test: raptor_cc.c libraptor.la
	$(LINK) $(DEFS) -static -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_cc.c libraptor.la $(LIBS)

raptor_uri_test: raptor_uri.c libraptor.la
	$(LINK) $(DEFS) -static -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_uri.c libraptor.la $(LIBS)

@SET_MAKE@

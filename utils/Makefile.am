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
bin_SCRIPTS = raptor-config
lib_LTLIBRARIES = libraptor.la

include_HEADERS = raptor.h ntriples.h

man_MANS = rdfdump.1 libraptor.3

rdfdump_SOURCES = rdfdump.c
rdfdump_LDADD = libraptor.la
if MAINTAINER_MODE
# easier to debug static
rdfdump_LDFLAGS = -static
endif
libraptor_la_SOURCES = raptor_parse.c ntriples_parse.c \
raptor_uri.c

libraptor_la_LDFLAGS = -version-info @RAPTOR_LIBTOOL_VERSION@


EXTRA_DIST=ChangeLog \
README      NEWS      LICENSE.txt \
README.html NEWS.html LICENSE.html \
INSTALL.html MPL.html \
raptor-config.in \
autogen.sh \
raptor.spec.in \
dc.rdf \
raptor_cc.gperf \
raptor_cc.c \
$(man_MANS)


SUBDIRS= debian tests win32

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
	 gperf --struct-type --duplicates --language=ANSI-C --readonly-tables --delimiters=\; $? > $@


# Some people need a little help ;-)
test: check

raptor_cc_test: $(srcdir)/raptor_cc.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -DSTANDALONE -static $(srcdir)/raptor_cc.c libraptor.la $(LIBS)

raptor_uri_test: $(srcdir)/raptor_uri.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -DSTANDALONE -static $(srcdir)/raptor_uri.c libraptor.la $(LIBS)

finish: libraptor.la
	rm -rf $(LOCAL_LIB_DIR)
	mkdir $(LOCAL_LIB_DIR)
	lib=`cd $(LOCAL_LIB_DIR); pwd`; \
	$(LIBTOOL) --mode=install install libraptor.la $$lib; \
	$(LIBTOOL) --mode=finish $$lib

deb: distcheck
	rm -rf $(distdir)
	tar xfz $(distdir).tar.gz
	cd $distdir && dpkg-buildpackage -rfakeroot

@SET_MAKE@

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
lib_LIBRARIES = libraptor.a

include_HEADERS = raptor.h ntriples.h

# RDF/XML parser - Raptor
rdfdump_LDADD = libraptor.a

libraptor_a_SOURCES = raptor_parse.c ntriples_parse.c

libraptor_a_LIBADD = @XML_OBJS@
libraptor_a_DEPENDENCIES = @XML_OBJS@

EXTRA_libraptor_a_SOURCES = rdfdump.c


EXTRA_DIST=ChangeLog \
README      NEWS      LICENSE.txt \
README.html NEWS.html LICENSE.html \
INSTALL.html MPL.html \
autogen.sh \
raptor.spec.in

SUBDIRS= \
	tests \
	win32

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

@SET_MAKE@

# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Rapier
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
lib_LIBRARIES = librapier.a

include_HEADERS = rapier.h

rdfdump_LDADD = librapier.a

librapier_a_SOURCES = rapier_parse.c

librapier_a_LIBADD = @XML_OBJS@
librapier_a_DEPENDENCIES = @XML_OBJS@

EXTRA_librapier_a_SOURCES = rdfdump.c

RDF_TEST_FILES=ex-0.rdf ex-1.rdf ex-2.rdf ex-3.rdf ex-4.rdf ex-5.rdf \
ex-6.rdf ex-7.rdf ex-8.rdf

EXTRA_DIST=ChangeLog \
README      NEWS      LICENSE.txt \
README.html NEWS.html LICENSE.html \
INSTALL.html MPL.html \
autogen.sh \
$(RDF_TEST_FILES) \
rapier.spec.in

# Why is this not in the default makefile?
CC=@CC@

# Memory debugging alternatives
MEM=@MEM@
MEM_LIBS=@MEM_LIBS@

# 1) None (use standard functions directly)
#MEM=
#MEM_LIBS=

# 2) Use dmalloc library
#MEM=-DRAPIER_MEMORY_DEBUG_DMALLOC=1
#MEM_LIBS=-ldmalloc


CPPFLAGS=@CPPFLAGS@ $(MEM)
STANDARD_CFLAGS=@STANDARD_CFLAGS@ $(MEM)
LIBS=@LIBS@ $(MEM_LIBS)


# Create some text files from HTML sources
LYNX=lynx
LYNXARGS=-dump -nolist

SUFFIXES = .html .txt

.html.txt:
	$(LYNX) $(LYNXARGS) $< > $@

README: README.html
	$(LYNX) $(LYNXARGS) $< > $@

NEWS: NEWS.html
	$(LYNX) $(LYNXARGS) $< > $@

@SET_MAKE@

check-local: rdfdump
	@for test in $(RDF_TEST_FILES); do \
	  echo Checking $(srcdir)/$$test; \
	  rdfdump file:$(srcdir)/$$test; \
	done

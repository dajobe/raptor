# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor utils
#
# Copyright (C) 2000-2009, David Beckett http://www.dajobe.org/
# Copyright (C) 2000-2004, University of Bristol, UK http://www.bristol.ac.uk/
# 
# This package is Free Software and part of Redland http://librdf.org/
# 
# It is licensed under the following three licenses as alternatives:
#   1. GNU Lesser General Public License (LGPL) V2.1 or any newer version
#   2. GNU General Public License (GPL) V2 or any newer version
#   3. Apache License, V2.0 or any newer version
# 
# You may not use this file except in compliance with at least one of
# the above three licenses.
# 
# See LICENSE.html or LICENSE.txt at the top of this package for the
# complete terms and further detail along with the license texts for
# the licenses in COPYING.LIB, COPYING and LICENSE-2.0.txt respectively.
# 
# 


bin_PROGRAMS = rapper
noinst_PROGRAMS = rdfdiff

man_MANS = rapper.1

CLEANFILES = *.plist

# Memory debugging
MEM=@MEM@
MEM_LIBS=@MEM_LIBS@

AM_CPPFLAGS= $(MEM) -I$(top_srcdir)/src
LIBS=@LIBS@ $(MEM_LIBS)


ANALYZE = clang
ANALYZE_FLAGS = "--analyze"
# Based on COMPILE target
ANALYZE_COMMAND = $(ANALYZE) \
	$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	$(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS) \
	$(ANALYZE_FLAGS)


EXTRA_DIST= \
CMakeLists.txt \
rapper.html \
$(man_MANS)

rapper_SOURCES = rapper.c
if GETOPT
rapper_SOURCES += getopt.c raptor_getopt.h
endif
rapper_LDADD= $(top_builddir)/src/libraptor2.la

rdfdiff_SOURCES = rdfdiff.c
if GETOPT
rdfdiff_SOURCES += getopt.c raptor_getopt.h
endif
rdfdiff_LDADD= $(top_builddir)/src/libraptor2.la


if MAINTAINER_MODE
rapper.html: $(srcdir)/rapper.1 $(srcdir)/../scripts/fix-groff-xhtml
	-groff -man -Thtml -P-l $< | tidy -asxml -wrap 1000 2>/dev/null | $(PERL) $(srcdir)/../scripts/fix-groff-xhtml $@
endif

$(top_builddir)/src/libraptor2.la:
	cd $(top_builddir)/src && $(MAKE) libraptor2.la

if MAINTAINER_MODE
# Run Clang static analyzer over sources.
analyze: $(SOURCES)
	@list='$(SOURCES)'; \
	result=0; \
	for file in $$list; do \
	  if echo $$file | grep '\.c$$' >/dev/null 2>&1; then \
	    $(RECHO) "Analyzing $$file"; \
	    $(ANALYZE_COMMAND) $(srcdir)/$$file; \
	    status=$$?; \
	    if test $$status != 0; then \
	      result=1; \
	    fi; \
	  fi; \
	done; \
	set -e; exit $$result
endif

# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor libraptor
#
# Copyright (C) 2010, David Beckett http://www.dajobe.org/
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

EXTRA_DIST= \
fix-bison.pl \
fix-flex.pl \
fix-groff-xhtml \
fix-gtkdoc-header.pl \
process-changes.pl \
rdfcompare

CLEANFILES =

if MAINTAINER_MODE
EXTRA_PROGRAMS = build-formats

CLEANFILES += build-formats

build_formats_SOURCES = build-formats.c
build_formats_LDADD= $(top_builddir)/src/libraptor2.la

endif

$(top_builddir)/src/libraptor2.la:
	cd $(top_builddir)/src && $(MAKE) libraptor2.la

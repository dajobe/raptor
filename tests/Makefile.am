# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor tests
#
# Copyright (C) 2000-2008, David Beckett http://www.dajobe.org/
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

TESTS=raptor_empty_test$(EXEEXT)

AM_CPPFLAGS=-I$(top_srcdir)/src
AM_CFLAGS= -I$(top_builddir)/src @CFLAGS@ $(MEM)
AM_LDFLAGS=$(top_builddir)/src/libraptor.la $(MEM_LIBS)

EXTRA_PROGRAMS=$(TESTS)

CLEANFILES=$(TESTS)

raptor_empty_test_SOURCES=empty.c


# Used to make N-triples output consistent
BASE_URI=http://librdf.org/raptor/tests/

SUBDIRS = rdfxml ntriples turtle trig grddl rdfa feeds


$(top_builddir)/src/libraptor.la:
	cd $(top_builddir)/src && $(MAKE) $(AM_MAKEFLAGS) libraptor.la

# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor examples
#
# Copyright (C) 2003-2010, David Beckett http://www.dajobe.org/
# Copyright (C) 2003-2004, University of Bristol, UK http://www.bristol.ac.uk/
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

EXTRA_PROGRAMS = \
raptor_abort grapper \
rdfcat rdfprint rdfserialize rdfguess rdfcount

examples: $(EXTRA_PROGRAMS)

CLEANFILES = $(EXTRA_PROGRAMS)

AM_CFLAGS=$(MEM)
AM_LDFLAGS=$(MEM_LIBS)
AM_CPPFLAGS=-I$(top_srcdir)/src

raptor_abort_SOURCES = raptor_abort.c
raptor_abort_LDADD=$(top_builddir)/src/libraptor2.la
raptor_abort_DEPENDENCIES = $(top_builddir)/src/libraptor2.la

grapper_SOURCES = grapper.c
grapper_LDADD=$(top_builddir)/src/libraptor2.la
grapper_CFLAGS=`pkg-config --cflags gtk+-2.0 gconf-2.0`
grapper_LDFLAGS=`pkg-config --libs gtk+-2.0 gconf-2.0`
grapper_DEPENDENCIES = $(top_builddir)/src/libraptor2.la

rdfcat_SOURCES = rdfcat.c
rdfcat_LDADD=$(top_builddir)/src/libraptor2.la
rdfcat_DEPENDENCIES = $(top_builddir)/src/libraptor2.la

rdfprint_SOURCES = rdfprint.c
rdfprint_LDADD=$(top_builddir)/src/libraptor2.la
rdfprint_DEPENDENCIES = $(top_builddir)/src/libraptor2.la

rdfserialize_SOURCES = rdfserialize.c
rdfserialize_LDADD=$(top_builddir)/src/libraptor2.la
rdfserialize_DEPENDENCIES = $(top_builddir)/src/libraptor2.la

rdfguess_SOURCES = rdfguess.c
rdfguess_LDADD=$(top_builddir)/src/libraptor2.la
rdfguess_DEPENDENCIES = $(top_builddir)/src/libraptor2.la

rdfcount_SOURCES = rdfcount.c
rdfcount_LDADD=$(top_builddir)/src/libraptor2.la
rdfcount_DEPENDENCIES = $(top_builddir)/src/libraptor2.la


$(top_builddir)/src/libraptor2.la:
	cd $(top_builddir)/src && $(MAKE) libraptor2.la

$(top_builddir)/../librdf/librdf.la:
	cd $(top_builddir)/../librdf && $(MAKE) librdf.la

grapper-future:
	$(MAKE) clean
	$(MAKE) grapper CC="$(CC) -DGTK_DISABLE_DEPRECATED" V=1

# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor examples
#
# $Id$
#
# Copyright (C) 2003 David Beckett - http://purl.org/net/dajobe/
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

EXTRA_PROGRAMS = raptor_abort grapper

examples: grapper

CLEANFILES = $(EXTRA_PROGRAMS)

AM_CFLAGS=@CFLAGS@ $(MEM)
LIBS=@LIBS@ $(MEM_LIBS)
AM_CPPFLAGS=-I$(top_srcdir)

raptor_abort_SOURCES = raptor_abort.c
raptor_abort_LDADD=$(top_builddir)/libraptor.la
raptor_abort_DEPENDENCIES = $(top_builddir)/libraptor.la

grapper_SOURCES = grapper.c
grapper_LDADD=$(top_builddir)/libraptor.la
grapper_CFLAGS=$(shell pkg-config --cflags gtk+-2.0)
grapper_LDFLAGS=$(shell pkg-config --libs gtk+-2.0)
grapper_DEPENDENCIES = $(top_builddir)/libraptor.la



$(top_builddir)/libraptor.la:
	cd $(top_builddir) && $(MAKE) libraptor.la

$(top_builddir)/../librdf/librdf.la:
	cd $(top_builddir)/../librdf && $(MAKE) librdf.la

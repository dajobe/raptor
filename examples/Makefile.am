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

EXAMPLES=raptor_abort

# Need this so that automake doesn't moan
noinst_PROGRAMS = $(EXAMPLES)

CLEANFILES=$(EXAMPLES)

examples: $(EXAMPLES)

AM_CPPFLAGS=-I$(top_srcdir)
LDADD=$(top_builddir)/libraptor.la
LIBS=@LIBS@ @REDLAND_LIBS@

raptor_abort_SOURCES = raptor_abort.c


$(top_builddir)/libraptor.la:
	cd $(top_builddir) && $(MAKE) libraptor.la

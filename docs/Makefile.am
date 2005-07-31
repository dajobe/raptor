# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor docs
#
# $Id$
#
# Copyright (C) 2000-2005, David Beckett http://purl.org/net/dajobe/
# Institute for Learning and Research Technology http://www.ilrt.bristol.ac.uk/
# University of Bristol, UK http://www.bristol.ac.uk/
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


man_MANS = libraptor.3

EXTRA_DIST=\
libraptor.html \
$(man_MANS)

if MAINTAINER_MODE
libraptor.html: $(srcdir)/libraptor.3 $(srcdir)/../fix-groff-xhtml
	-groff -man -Thtml -P-l $< | tidy -asxml -wrap 1000 2>/dev/null | perl $(srcdir)/../fix-groff-xhtml $@

endif

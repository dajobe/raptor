# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor tests
#
# Copyright (C) 2000-2006, David Beckett http://purl.org/net/dajobe/
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

TURTLE_SERIALIZE_TEST_FILES=ex-62.rdf


# Used to make N-triples output consistent
BASE_URI=http://librdf.org/raptor/tests/

EXTRA_DIST = \
	$(TURTLE_SERIALIZE_TEST_FILES)

RDFXML_DIR = rdfxml
NTRIPLES_DIR = ntriples

SUBDIRS = $(RDFXML_DIR) $(NTRIPLES_DIR) turtle trig grddl rdfa


build-rapper:
	@(cd $(top_builddir)/utils ; $(MAKE) rapper$(EXEEXT))

build-rdfdiff:
	@(cd $(top_builddir)/utils ; $(MAKE) rdfdiff$(EXEEXT))

check-local: build-rapper \
check-turtle check-turtle-serialize


check-turtle: build-rapper
	@set +e; result=0; \
	$(ECHO) "Testing Turtle with N-Triples tests"; \
	NT_TEST_FILES=`cd $(srcdir)/$(NTRIPLES_DIR) && make print-nt-test-files | sed -e 's,^,$(NTRIPLES_DIR)/,'`; \
	for test in $$NT_TEST_FILES; do \
	  name=`basename $$test .nt` ; \
	  baseuri=-; \
	  $(ECHO) $(ECHO_N) "Checking $$test $(ECHO_C)"; \
	  $(top_builddir)/utils/rapper -q -i turtle -o ntriples -n file:$(srcdir)/$$test $$baseuri > $$name.res 2>/dev/null; \
	  test_expected=$(srcdir)/$(NTRIPLES_DIR)/$$name.out; \
	  if cmp $$test_expected $$name.res >/dev/null 2>&1; then \
	    $(ECHO) "ok"; \
	  else \
	    $(ECHO) "FAILED"; \
	    diff $$test_expected $$name.res; result=1; \
	  fi; \
	  rm -f $$name.res ; \
	done; \
	set -e; exit $$result

check-turtle-serialize: build-rdfdiff build-rapper $(TURTLE_SERIALIZE_TEST_FILES)
	@set +e; result=0; \
	RDF_TEST_FILES=`cd $(srcdir)/$(RDFXML_DIR) && make print-rdf-test-files | sed -e 's,^,$(RDFXML_DIR)/,'`; \
	$(ECHO) "Testing turtle serialization with legal rdf/xml"; \
	for test in $$RDF_TEST_FILES $(TURTLE_SERIALIZE_TEST_FILES); do \
	  name=`basename $$test .rdf` ; \
	  baseuri=-; \
	  $(ECHO) $(ECHO_N) "Checking $$test $(ECHO_C)"; \
	  $(top_builddir)/utils/rapper -q -o turtle $(srcdir)/$$test $$baseuri > $$name-turtle.rdf 2> $$name.err; \
	  status1=$$?; \
	  $(top_builddir)/utils/rdfdiff -t turtle $(srcdir)/$$test $$name-turtle.rdf > $$name.res 2> $$name.err; \
	  status2=$$?; \
	  if test $$status1 = 0 -a $$status2 = 0; then \
	    $(ECHO) "ok"; \
	  else \
	    $(ECHO) "FAILED"; \
	    echo "rapper + rdfdiff output:"; cat $$name.err; result=1; \
	  fi; \
	  rm -f $$name-turtle.rdf $$name.res $$name.err; \
	done; \
	set -e; exit $$result


$(top_builddir)/src/libraptor.la:
	cd $(top_builddir)/src && $(MAKE) $(AM_MAKEFLAGS) libraptor.la

# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor tests
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

RDF_TEST_FILES=ex-00.rdf ex-01.rdf ex-02.rdf ex-03.rdf ex-04.rdf ex-05.rdf \
ex-06.rdf ex-07.rdf ex-08.rdf ex-09.rdf ex-10.rdf ex-11.rdf ex-12.rdf \
ex-13.rdf ex-14.rdf ex-15.rdf ex-16.rdf ex-17.rdf ex-18.rdf ex-19.rdf \
ex-20.rdf ex-21.rdf ex-22.rdf ex-23.rdf ex-24.rdf ex-25.rdf ex-26.rdf \
ex-27.rdf ex-28.rdf ex-29.rdf ex-30.rdf ex-31.rdf \
22-rdf-syntax-ns.rdf rdfs-namespace.rdf rdf-schema.rdf

RDF_BAD_TEST_FILES=bad-00.rdf bad-01.rdf

RDF_OUT_FILES=ex-00.out ex-01.out ex-02.out ex-03.out ex-04.out ex-05.out \
ex-06.out ex-07.out ex-08.out ex-09.out ex-10.out ex-11.out ex-12.out \
ex-13.out ex-14.out ex-15.out ex-16.out ex-17.out ex-18.out ex-19.out \
ex-20.out ex-21.out ex-22.out ex-23.out ex-24.out ex-25.out ex-26.out \
ex-27.out ex-28.out ex-29.out ex-30.out ex-31.out \
22-rdf-syntax-ns.out rdfs-namespace.out rdf-schema.out

RDF_TEST_ANSWER_FILES=ex-13.nt

NT_TEST_FILES=test.nt

NT_OUT_FILES=test.out

# Used to make N-triples output consistent
BASE_URI=http://www.redland.opensource.ac.uk/raptor/tests/
# for 22-rdf-syntax-ns.rdf
RDF_NS_URI=http://www.w3.org/1999/02/22-rdf-syntax-ns
# for rdfs-namespace.rdf (2002-04-30) and rdf-schema.rdf (2000-03-27)
RDFS_NS_URI=http://www.w3.org/2000/01/rdf-schema

EXTRA_DIST = \
	$(RDF_TEST_FILES) \
	$(RDF_TEST_ANSWER_FILES) \
	$(RDF_BAD_TEST_FILES) \
	$(RDF_OUT_FILES) \
	$(NT_TEST_FILES) \
	$(NT_OUT_FILES)


$(top_builddir)/rdfdump:
	@(cd .. ; $(MAKE) rdfdump)

check-local: $(top_builddir)/rdfdump $(RDF_TEST_FILES) $(NT_TEST_FILES)
	result=0; for test in $(RDF_TEST_FILES); do \
	  name=`basename $$test .rdf` ; \
	  if test $$name = 22-rdf-syntax-ns; then \
	    baseuri=$(RDF_NS_URI); \
	  elif test $$name = rdfs-namespace -o $$name = rdf-schema; then \
	    baseuri=$(RDFS_NS_URI); \
	  else \
	    baseuri=$(BASE_URI)$$test; \
	  fi; \
	  echo -n "Checking $$test "; \
	  $(top_builddir)/rdfdump -q -o ntriples file:$(srcdir)/$$test $$baseuri > $$name.res 2>/dev/null; \
	  if cmp $(srcdir)/$$name.out $$name.res >/dev/null 2>&1; then \
	    echo ok; \
	  else \
	    echo FAILED; diff $(srcdir)/$$name.out $$name.res; result=1; \
	  fi; \
	  rm -f $$name.res ; \
	done; \
	for test in $(RDF_BAD_TEST_FILES); do \
	  name=`basename $$test .rdf` ; \
	  baseuri=$(BASE_URI)$$test; \
	  echo -n "Checking $$test "; \
	  $(top_builddir)/rdfdump -q -o ntriples file:$(srcdir)/$$test $$baseuri > $$name.res 2>/dev/null; \
	  status=$$?; \
	  if test $$status != 0 ; then \
	    echo ok; \
	  else \
	    echo FAILED - parsing should have failed but returned $$status; \
	    cat $$name.res; result=1; \
	  fi; \
	  rm -f $$name.res ; \
	done; \
	for test in $(NT_TEST_FILES); do \
	  name=`basename $$test .nt` ; \
	  echo -n "Checking $$test "; \
	  $(top_builddir)/rdfdump -q -o ntriples -n file:$(srcdir)/$$test $(BASE_URI)$$test > $$name.res 2>/dev/null; \
	  if cmp $(srcdir)/$$name.out $$name.res >/dev/null 2>&1; then \
	    echo ok; \
	  else \
	    echo FAILED; diff $(srcdir)/$$name.out $$name.res; result=1; \
	  fi; \
	  rm -f $$name.res ; \
	done; \
	exit $$result

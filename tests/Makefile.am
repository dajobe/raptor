# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor tests
#
# $Id$
#
# Copyright (C) 2000-2003 David Beckett - http://purl.org/net/dajobe/
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

# These are errors in strict mode, warnings in lax - for now
RDF_BAGID_TEST_FILES=ex-03.rdf ex-42.rdf ex-43.rdf ex-44.rdf ex-45.rdf

RDF_TEST_FILES=ex-00.rdf ex-01.rdf ex-02.rdf ex-04.rdf ex-05.rdf \
ex-06.rdf ex-07.rdf ex-08.rdf ex-09.rdf ex-10.rdf ex-11.rdf ex-12.rdf \
ex-13.rdf ex-14.rdf ex-15.rdf ex-16.rdf ex-17.rdf ex-18.rdf \
ex-20.rdf ex-21.rdf ex-22.rdf ex-23.rdf ex-24.rdf ex-25.rdf ex-26.rdf \
ex-27.rdf ex-28.rdf ex-29.rdf ex-30.rdf ex-31.rdf ex-32.rdf ex-33.rdf \
ex-34.rdf ex-35.rdf ex-36.rdf ex-37.rdf ex-38.rdf ex-39.rdf ex-40.rdf \
ex-41.rdf ex-46.rdf ex-47.rdf \
ex-48.rdf ex-49.rdf \
22-rdf-syntax-ns.rdf rdfs-namespace.rdf rdf-schema.rdf \
owl-schema.rdf

RDF_BAD_TEST_FILES=bad-00.rdf bad-01.rdf bad-02.rdf bad-03.rdf \
bad-04.rdf bad-05.rdf bad-06.rdf bad-07.rdf bad-08.rdf bad-09.rdf \
bad-10.rdf bad-11.rdf bad-12.rdf bad-13.rdf bad-14.rdf

RDF_WARN_TEST_FILES=warn-00.rdf warn-01.rdf warn-02.rdf \
$(RDF_BAGID_TEST_FILES)

RDF_ASSUME_TEST_FILES=ex-19.rdf

RDF_OUT_FILES=ex-00.out ex-01.out ex-02.out ex-03.out ex-04.out ex-05.out \
ex-06.out ex-07.out ex-08.out ex-09.out ex-10.out ex-11.out ex-12.out \
ex-13.out ex-14.out ex-15.out ex-16.out ex-17.out ex-18.out \
ex-20.out ex-21.out ex-22.out ex-23.out ex-24.out ex-25.out ex-26.out \
ex-27.out ex-28.out ex-29.out ex-30.out ex-31.out ex-32.out ex-33.out \
ex-34.out ex-35.out ex-36.out ex-37.out ex-38.out ex-39.out ex-40.out \
ex-41.out ex-42.out ex-43.out ex-44.out ex-45.out ex-46.out ex-47.out \
ex-48.out ex-49.out \
22-rdf-syntax-ns.out rdfs-namespace.out rdf-schema.out \
owl-schema.out

RDF_WARN_OUT_FILES=warn-00.out warn-01.out warn-02.out

RDF_ASSUME_OUT_FILES=ex-19.out

NT_TEST_FILES=test.nt

NT_OUT_FILES=test.out

# Used to make N-triples output consistent
BASE_URI=http://www.redland.opensource.ac.uk/raptor/tests/
# for 22-rdf-syntax-ns.rdf
RDF_NS_URI=http://www.w3.org/1999/02/22-rdf-syntax-ns
# for rdfs-namespace.rdf (2002-04-30) and rdf-schema.rdf (2000-03-27)
RDFS_NS_URI=http://www.w3.org/2000/01/rdf-schema

OWL_DOC_URI="http://www.w3.org/2002/07/owl"

EXTRA_DIST = \
	$(RDF_TEST_FILES) \
	$(RDF_ASSUME_TEST_FILES) \
	$(RDF_BAD_TEST_FILES) \
	$(RDF_WARN_TEST_FILES) \
	$(RDF_OUT_FILES) \
	$(RDF_ASSUME_OUT_FILES) \
	$(RDF_WARN_OUT_FILES) \
	$(NT_TEST_FILES) \
	$(NT_OUT_FILES)


$(top_builddir)/rapper:
	@(cd $(top_builddir) ; $(MAKE) rapper)

check-local: check-rdf check-assume-rdf check-bad-rdf check-warn-rdf check-nt

check-rdf: $(top_builddir)/rapper $(RDF_TEST_FILES)
	@result=0; \
	$(ECHO) "Testing legal rdf/xml"; \
	for test in $(RDF_TEST_FILES); do \
	  name=`basename $$test .rdf` ; \
	  if test $$name = 22-rdf-syntax-ns; then \
	    baseuri=$(RDF_NS_URI); \
	  elif test $$name = rdfs-namespace -o $$name = rdf-schema; then \
	    baseuri=$(RDFS_NS_URI); \
	  elif test $$name = owl-schema; then \
	    baseuri=$(OWL_DOC_URI); \
	  else \
	    baseuri=$(BASE_URI)$$name.rdf; \
	  fi; \
	  $(ECHO) $(ECHO_N) "Checking $$test $(ECHO_C)"; \
	  $(top_builddir)/rapper -q -o ntriples file:$(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status != 0 -a $$status != 2 ; then \
	    $(ECHO) FAILED returned status $$status; result=1; \
	  elif cmp $(srcdir)/$$name.out $$name.res >/dev/null 2>&1; then \
	    if test $$status = 2 ; then \
	      $(ECHO) ok with warnings; grep Warning $$name.err; \
	    else \
	      $(ECHO) ok; \
	    fi; \
	  else \
	    $(ECHO) FAILED; diff $(srcdir)/$$name.out $$name.res; result=1; \
	  fi; \
	  rm -f $$name.res $$name.err; \
	done; \
	exit $$result

check-assume-rdf: $(top_builddir)/rapper $(RDF_ASSUME_TEST_FILES)
	@result=0; \
	$(ECHO) "Testing rdf/xml known by context"; \
	for test in $(RDF_ASSUME_TEST_FILES); do \
	  name=`basename $$test .rdf` ; \
	  baseuri=$(BASE_URI)$$name.rdf; \
	  $(ECHO) $(ECHO_N) "Checking $$test $(ECHO_C)"; \
	  $(top_builddir)/rapper -q -a -o ntriples file:$(srcdir)/$$test $$baseuri > $$name.res 2>/dev/null; \
	  if cmp $(srcdir)/$$name.out $$name.res >/dev/null 2>&1; then \
	    $(ECHO) ok; \
	  else \
	    $(ECHO) FAILED; diff $(srcdir)/$$name.out $$name.res; result=1; \
	  fi; \
	  rm -f $$name.res ; \
	done; \
	exit $$result

check-bad-rdf: $(top_builddir)/rapper $(RDF_BAD_TEST_FILES)
	@set +e; result=0; \
	$(ECHO) "Testing bad rdf/xml fails"; \
	for test in $(RDF_BAD_TEST_FILES); do \
	  name=`basename $$test .rdf` ; \
	  baseuri=$(BASE_URI)$$name.rdf; \
	  $(ECHO) $(ECHO_N) "Checking $$test $(ECHO_C)"; \
	  $(top_builddir)/rapper -q -o ntriples file:$(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status -eq 1 ; then \
	    $(ECHO) ok; \
	  elif test $$status -eq 2 ; then \
	    $(ECHO) FAILED - parsing succeeded with a warning; \
	    cat $$name.res; grep Warning $$name.err; result=1; \
	  else \
	    $(ECHO) FAILED - parsing failed with unknown status $$status; \
	    cat $$name.res; result=1; \
	  fi; \
	  rm -f $$name.res $$name.err ; \
	done; \
	set -e; exit $$result

check-warn-rdf: $(top_builddir)/rapper $(RDF_WARN_TEST_FILES)
	@set +e; result=0; \
	$(ECHO) "Testing rdf/xml with warnings"; \
	for test in $(RDF_WARN_TEST_FILES); do \
	  name=`basename $$test .rdf` ; \
	  baseuri=$(BASE_URI)$$name.rdf; \
	  $(ECHO) $(ECHO_N) "Checking $$test $(ECHO_C)"; \
	  $(top_builddir)/rapper -q -o ntriples file:$(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status -eq 1 ; then \
	    $(ECHO) FAILED - parsing failed when should have warned; \
	    cat $$name.res; grep Error $$name.err; result=1; \
	  elif test $$status -eq 2 ; then \
	    $(ECHO) ok; \
	  else \
	    $(ECHO) FAILED - parsing failed with unknown status $$status; \
	    cat $$name.res; result=1; \
	  fi; \
	  rm -f $$name.res $$name.err ; \
	done; \
	set -e; exit $$result

check-nt: $(top_builddir)/rapper $(NT_TEST_FILES)
	@result=0; \
	$(ECHO) "Testing N-Triples"; \
	for test in $(NT_TEST_FILES); do \
	  name=`basename $$test .nt` ; \
	  $(ECHO) $(ECHO_N) "Checking $$test $(ECHO_C)"; \
	  $(top_builddir)/rapper -q -o ntriples -n file:$(srcdir)/$$test $(BASE_URI)$$test > $$name.res 2>/dev/null; \
	  if cmp $(srcdir)/$$name.out $$name.res >/dev/null 2>&1; then \
	    $(ECHO) ok; \
	  else \
	    $(ECHO) FAILED; diff $(srcdir)/$$name.out $$name.res; result=1; \
	  fi; \
	  rm -f $$name.res ; \
	done; \
	exit $$result

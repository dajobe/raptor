# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor Turtle tests
#
# $Id$
#
# Copyright (C) 2003-2004 David Beckett - http://purl.org/net/dajobe/
# Institute for Learning and Research Technology - http://www.ilrt.bris.ac.uk/
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

TEST_FILES=test-00.ttl test-01.ttl test-02.ttl test-03.ttl \
test-04.ttl test-05.ttl test-06.ttl test-07.ttl test-08.ttl \
test-09.ttl test-10.ttl test-11.ttl \
rdfq-results.ttl

TEST_BAD_FILES=bad-00.ttl bad-01.ttl bad-02.ttl bad-03.ttl \
bad-04.ttl

TEST_WARN_FILES=

TEST_OUT_FILES=test-00.out test-01.out test-02.out test-03.out \
test-04.out test-05.out test-06.out test-07.out test-08.out \
test-09.out test-10.out test-11.out \
rdfq-results.out

TEST_WARN_OUT_FILES=

# Used to make N-triples output consistent
BASE_URI=http://www.redland.opensource.ac.uk/raptor/tests/turtle/


EXTRA_DIST = \
	$(TEST_FILES) \
	$(TEST_BAD_FILES) \
	$(TEST_WARN_FILES) \
	$(TEST_OUT_FILES) \
	$(TEST_WARN_OUT_FILES)

$(top_builddir)/rapper:
	@(cd $(top_builddir) ; $(MAKE) rapper)

check-local: check-rdf check-bad-rdf
# check-warn-rdf

check-rdf: $(top_builddir)/rapper $(TEST_FILES)
	@result=0; \
	$(ECHO) "Testing legal Turtle"; \
	for test in $(TEST_FILES); do \
	  name=`basename $$test .ttl` ; \
	  baseuri=$(BASE_URI)$$test; \
	  $(ECHO) $(ECHO_N) "Checking $$test $(ECHO_C)"; \
	  $(top_builddir)/rapper -q -i turtle -o ntriples $(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status != 0 -a $$status != 2 ; then \
	    $(ECHO) FAILED returned status $$status; result=1; \
	  elif cmp $(srcdir)/$$name.out $$name.res >/dev/null 2>&1; then \
	    if test $$status = 2 ; then \
	      $(ECHO) "ok with warnings"; grep Warning $$name.err; \
	    else \
	      $(ECHO) "ok"; \
	    fi; \
	  else \
	    $(ECHO) "FAILED"; \
	    diff $(srcdir)/$$name.out $$name.res; result=1; \
	  fi; \
	  rm -f $$name.res $$name.err; \
	done; \
	exit $$result

check-bad-rdf: $(top_builddir)/rapper $(TEST_BAD_FILES)
	@set +e; result=0; \
	$(ECHO) "Testing bad Turtle fails"; \
	for test in $(TEST_BAD_FILES); do \
	  name=`basename $$test .ttl` ; \
	  baseuri=$(BASE_URI)$$test; \
	  $(ECHO) $(ECHO_N) "Checking $$test $(ECHO_C)"; \
	  $(top_builddir)/rapper -q -i turtle -o ntriples file:$(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status -eq 1 ; then \
	    $(ECHO) "ok"; \
	  elif test $$status -eq 2 ; then \
	    $(ECHO) "FAILED - parsing succeeded with a warning"; \
	    cat $$name.res; grep Warning $$name.err; result=1; \
	  elif test $$status -eq 0 ; then \
	    $(ECHO) "FAILED - parsing succeeded but should have failed"; \
	    cat $$name.res; result=1; \
	  else \
	    $(ECHO) "FAILED - parsing failed with unknown status $$status"; \
	    cat $$name.res; result=1; \
	  fi; \
	  rm -f $$name.res $$name.err ; \
	done; \
	set -e; exit $$result

check-warn-rdf: $(top_builddir)/rapper $(TEST_WARN_FILES)
	@set +e; result=0; \
	$(ECHO) "Testing Turtle with warnings"; \
	for test in $(TEST_WARN_FILES); do \
	  name=`basename $$test .ttl` ; \
	  baseuri=$(BASE_URI)$$test; \
	  $(ECHO) $(ECHO_N) "Checking $$test $(ECHO_C)"; \
	  $(top_builddir)/rapper -q -i turtle -o ntriples file:$(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status -eq 1 ; then \
	    $(ECHO) "FAILED - parsing failed when should have warned"; \
	    cat $$name.res; grep Error $$name.err; result=1; \
	  elif test $$status -eq 2 ; then \
	    $(ECHO) "ok"; \
	  else \
	    $(ECHO) "FAILED - parsing failed with unknown status $$status"; \
	    cat $$name.res; result=1; \
	  fi; \
	  rm -f $$name.res $$name.err ; \
	done; \
	set -e; exit $$result

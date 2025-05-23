# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor Turtle tests
#
# Copyright (C) 2003-2009, David Beckett http://www.dajobe.org/
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

RDFXML_DIR = ../rdfxml
NTRIPLES_DIR = ../ntriples

TEST_FILES=test-00.ttl test-01.ttl test-02.ttl test-03.ttl \
test-04.ttl test-05.ttl test-06.ttl test-07.ttl test-08.ttl \
test-09.ttl test-10.ttl test-11.ttl test-12.ttl test-13.ttl \
test-14.ttl test-15.ttl test-16.ttl test-17.ttl test-18.ttl \
test-19.ttl test-20.ttl test-21.ttl test-22.ttl test-23.ttl \
test-24.ttl test-25.ttl test-26.ttl test-27.ttl \
test-29.ttl test-30.ttl test-33.ttl test-36.ttl test-37.ttl \
test-38.ttl test-40.ttl \
bad-15.ttl bad-17.ttl bad-18.ttl bad-21.ttl bad-22.ttl \
rdf-schema.ttl \
rdfs-namespace.ttl \
rdfq-results.ttl

TEST_BAD_FILES=bad-00.ttl bad-01.ttl bad-02.ttl bad-03.ttl \
bad-04.ttl bad-05.ttl bad-06.ttl bad-07.ttl bad-08.ttl bad-09.ttl \
bad-10.ttl bad-11.ttl bad-12.ttl bad-13.ttl bad-14.ttl bad-16.ttl \
bad-19.ttl bad-20.ttl \
bad-23.ttl bad-24.ttl

TEST_OUT_FILES=test-00.out test-01.out test-02.out test-03.out \
test-04.out test-05.out test-06.out test-07.out test-08.out \
test-09.out test-10.out test-11.out test-12.out test-13.out \
test-14.out test-15.out test-16.out test-17.out test-18.out \
test-19.out test-20.out test-21.out test-22.out test-23.out \
test-24.out test-25.out test-26.out test-27.out test-28.out \
test-29.out test-30.out test-33.out test-36.out test-37.out \
test-38.out test-40.out \
bad-15.out bad-17.out bad-18.out bad-21.out bad-22.out \
rdf-schema.out \
rdfs-namespace.out \
rdfq-results.out

TEST_SERIALIZE_FILES= \
test-28.ttl test-31.ttl test-32.ttl test-34.ttl test-39.ttl

TEST_SERIALIZE_OUT_FILES= \
test-28-out.ttl test-31-out.ttl test-32-out.ttl test-34-out.ttl test-39-out.ttl

TURTLE_HACK_OUT_FILES= \
ex-38-turtle.out

TURTLE_SERIALIZE_RDF_FILES=ex-62.rdf

TEST_MANIFEST_FILES=manifest.ttl manifest-bad.ttl

# Used to make N-triples output consistent
BASE_URI=http://www.w3.org/2001/sw/DataAccess/df1/tests/
# for rdf-schema.ttl
RDF_NS_URI=http://www.w3.org/1999/02/22-rdf-syntax-ns
# for rdfs-namespace.ttl
RDFS_NS_URI=http://www.w3.org/2000/01/rdf-schema

ALL_TEST_FILES= README.txt \
	$(TEST_FILES) \
	$(TEST_BAD_FILES) \
	$(TEST_OUT_FILES) \
	$(TEST_MANIFEST_FILES) \
	$(TEST_SERIALIZE_FILES) \
	$(TEST_SERIALIZE_OUT_FILES) \
	$(TURTLE_SERIALIZE_RDF_TEST_FILES) \
	$(TURTLE_SERIALIZE_RDF_FILES) \
	$(TURTLE_HACK_OUT_FILES)

EXTRA_DIST = $(ALL_TEST_FILES)

RAPPER  = $(top_builddir)/utils/rapper
RDFDIFF = $(top_builddir)/utils/rdfdiff

build-rapper:
	@(cd $(top_builddir)/utils ; $(MAKE) rapper$(EXEEXT))

build-rdfdiff:
	@(cd $(top_builddir)/utils ; $(MAKE) rdfdiff$(EXEEXT))

check-local: check-rdf check-bad-rdf check-turtle-serialize \
check-turtle-serialize-syntax check-turtle-parse-ntriples \
check-turtle-serialize-rdf

if MAINTAINER_MODE
check_rdf_deps = $(TEST_FILES)
endif

check-rdf: build-rapper $(check_rdf_deps)
	@set +e; result=0; \
	$(RECHO) "Testing legal Turtle"; \
	base="$@"; \
	for test in $(TEST_FILES); do \
	  name=`basename $$test .ttl` ; \
	  baseuri=$(BASE_URI)$$test; \
	  expected="$(srcdir)/$$name.out"; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i turtle -o ntriples $(srcdir)/$$test $$baseuri > $$base.res 2> $$base.err; \
	  status=$$?; \
	  if test $$status != 0 -a $$status != 2 ; then \
	    $(RECHO) "FAILED returned status $$status"; result=1; \
	    $(RECHO) $(RAPPER) -q -i turtle -o ntriples $(srcdir)/$$test $$baseuri '>' $$base.res; \
	    cat $$base.err; \
	  elif cmp $$expected $$base.res >/dev/null 2>&1; then \
	    if test $$status = 2 ; then \
	      $(RECHO) "ok with warnings"; grep Warning $$base.err; \
	    else \
	      $(RECHO) "ok"; \
	    fi; \
	  else \
	    $(RECHO) "FAILED"; result=1; \
	    $(RECHO) $(RAPPER) -q -i turtle -o ntriples $(srcdir)/$$test $$baseuri '>' $$base.res; \
	    cat $$base.err; \
	    diff $$expected $$base.res; \
	  fi; \
	done; \
	rm -f $$base.res $$base.err; \
	set -e; exit $$result

if MAINTAINER_MODE
check_bad_rdf_deps = $(TEST_BAD_FILES)
endif

check-bad-rdf: build-rapper $(check_bad_rdf_deps)
	@set +e; result=0; \
	$(RECHO) "Testing bad Turtle fails"; \
	base="$@"; \
	for test in $(TEST_BAD_FILES); do \
	  name=`basename $$test .ttl` ; \
	  baseuri=$(BASE_URI)$$test; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i turtle -o ntriples file:$(srcdir)/$$test $$baseuri > $$base.res 2> $$base.err; \
	  status=$$?; \
	  if test $$status -eq 1 ; then \
	    $(RECHO) "ok"; \
	  elif test $$status -eq 2 ; then \
	    $(RECHO) "FAILED - parsing succeeded with a warning";  result=1; \
	    $(RECHO) $(RAPPER) -q -i turtle -o ntriples file:$(srcdir)/$$test $$baseuri '>' $$base.res; \
	    cat $$base.res; grep Warning $$base.err; \
	  elif test $$status -eq 0 ; then \
	    $(RECHO) "FAILED - parsing succeeded but should have failed"; result=1; \
	    $(RECHO) $(RAPPER) -q -i turtle -o ntriples file:$(srcdir)/$$test $$baseuri '>' $$base.res; \
	    cat $$base.res; \
	  else \
	    $(RECHO) "FAILED - parsing failed with unknown status $$status"; result=1; \
	    $(RECHO) $(RAPPER) -q -i turtle -o ntriples file:$(srcdir)/$$test $$baseuri '>' $$base.res; \
	    cat $$base.err; \
	    cat $$base.res; \
	  fi; \
	done; \
	rm -f $$base.res $$base.err ; \
	set -e; exit $$result

if MAINTAINER_MODE
check_turtle_serialize_deps = $(TEST_FILES)
endif

check-turtle-serialize: build-rdfdiff build-rapper $(check_turtle_serialize_deps)
	@set +e; result=0; \
	$(RECHO) "Testing turtle serialization with legal turtle"; \
	base="$@"; \
	for test in $(TEST_FILES); do \
	  name=`basename $$test .ttl` ; \
	  if test $$name = rdf-schema; then \
	    baseuri=$(RDF_NS_URI); \
	  elif test $$name = rdfs-namespace; then \
	    baseuri=$(RDFS_NS_URI); \
	  else \
	    baseuri=$(BASE_URI)$$test; \
	  fi; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i turtle -o turtle $(srcdir)/$$test $$baseuri > $$base-turtle.ttl 2> $$base.err; \
	  status1=$$?; \
	  $(RDFDIFF) -f turtle -u $$baseuri -t turtle $(srcdir)/$$test $$base-turtle.ttl > $$base.res 2> $$base.err; \
	  status2=$$?; \
	  if test $$status1 = 0 -a $$status2 = 0; then \
	    $(RECHO) "ok"; \
	  else \
	    $(RECHO) "FAILED"; result=1; \
	    $(RECHO) $(RAPPER) -q -i turtle -o turtle $(srcdir)/$$test $$baseuri '>' $$base-turtle.ttl; \
	    $(RECHO) $(RDFDIFF) -f turtle -u $$baseuri -t turtle $(srcdir)/$$test $$base-turtle.ttl '>' $$base.res; \
	    cat $$base-turtle.ttl; cat $$base.err; \
	  fi; \
	done; \
	rm -f $$base-turtle.ttl $$base.res $$base.err; \
	set -e; exit $$result

if MAINTAINER_MODE
check_turtle_serialize_syntax_deps = $(TEST_SERIALIZE_FILES)
endif

check-turtle-serialize-syntax: build-rapper $(check_turtle_serialize_syntax_deps)
	@set +e; result=0; \
	$(RECHO) "Testing turtle exact serialization output"; \
	base="$@"; \
	for test in $(TEST_SERIALIZE_FILES); do \
	  name=`basename $$test .ttl` ; \
	  if test $$name = rdf-schema; then \
	    baseuri=$(RDF_NS_URI); \
	  elif test $$name = rdfs-namespace; then \
	    baseuri=$(RDFS_NS_URI); \
	  elif test $$name = test-31 -o $$name = test-32 -o $$name = test-39; then \
	    baseuri=-; \
	  else \
	    baseuri=$(BASE_URI)$$test; \
	  fi; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i turtle -o turtle $(srcdir)/$$test $$baseuri > $$base-turtle.ttl 2> $$base.err; \
	  cmp -s $(srcdir)/$$name-out.ttl $$base-turtle.ttl; \
	  status=$$?; \
	  if test $$status = 0; then \
	    $(RECHO) "ok"; \
	  else \
	    $(RECHO) "FAILED"; result=1; \
	    $(RECHO) $(RAPPER) -q -i turtle -o turtle $(srcdir)/$$test $$baseuri '>' $$base-turtle.ttl; \
	    cat $$base.err; \
	    diff -a -u $(srcdir)/$$name-out.ttl $$base-turtle.ttl; \
	  fi; \
	done; \
	rm -f $$base-turtle.ttl $$base.err; \
	set -e; exit $$result

check-turtle-parse-ntriples: build-rapper
	@set +e; result=0; \
	$(RECHO) "Testing Turtle parsing with N-Triples tests"; \
	NT_TEST_FILES=`unset MAKELEVEL MAKEFLAGS; cd $(NTRIPLES_DIR) && $(MAKE) print-nt-test-files | sed -e 's,^, ,' -e 's, ,$(NTRIPLES_DIR)/,g'`; \
	base="$@"; \
	for test in $$NT_TEST_FILES; do \
	  name=`basename $$test .nt` ; \
	  baseuri=-; \
	  expected=$(srcdir)/$(NTRIPLES_DIR)/$$name.out; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i turtle -o ntriples file:$(srcdir)/$$test $$baseuri > $$base.res 2> $$base.err; \
	  if cmp $$expected $$base.res >/dev/null 2>&1; then \
	    $(RECHO) "ok"; \
	  else \
	    $(RECHO) "FAILED";  result=1; \
	    $(RECHO) $(RAPPER) -q -i turtle -o ntriples file:$(srcdir)/$$test $$baseuri '>' $$base.res; \
	    cat $$base.err; \
	    diff $$expected $$base.res; \
	  fi; \
	done; \
	rm -f $$base.res $$base.err; \
	set -e; exit $$result

check-turtle-serialize-rdf: build-rdfdiff build-rapper
	@set +e; result=0; \
	RDF_TEST_FILES=`unset MAKELEVEL MAKEFLAGS; cd $(RDFXML_DIR) && $(MAKE) print-rdf-test-files | sed -e 's,^, ,' -e 's, ,$(RDFXML_DIR)/,g'`; \
	$(RECHO) "Testing turtle serialization with legal rdf/xml"; \
	base="$@"; \
	for test in $$RDF_TEST_FILES $(TURTLE_SERIALIZE_RDF_FILES); do \
	  name=`basename $$test .rdf` ; \
	  baseuri=-; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -o turtle $(srcdir)/$$test $$baseuri > $$base-turtle.ttl 2> $$base.err; \
	  status1=$$?; \
	  if test $$test = "$(RDFXML_DIR)/ex-38.rdf"; then \
	    diff $(srcdir)/ex-38-turtle.out $$base-turtle.ttl > $$base.res 2> $$base.err; \
	    status2=$$?; \
	  else \
	    $(RDFDIFF) -t turtle $(srcdir)/$$test $$base-turtle.ttl > $$base.res 2> $$base.err; \
	    status2=$$?; \
	  fi; \
	  if test $$status1 = 0 -a $$status2 = 0; then \
	    $(RECHO) "ok"; \
	  else \
	    $(RECHO) "FAILED"; result=1; \
	    $(RECHO) $(RAPPER) -q -o turtle $(srcdir)/$$test $$baseuri '>' $$base-turtle.ttl; \
	    $(RECHO) $(RDFDIFF) -t turtle $(srcdir)/$$test $$base-turtle.ttl '>' $$base.res; \
	    cat $$base.err; \
	  fi; \
	done; \
	rm -f $$base-turtle.ttl $$base.res $$base.err; \
	set -e; exit $$result


if MAINTAINER_MODE
zip: tests.zip

tests.zip: $(ALL_TEST_FILES)
	rm -f $@
	zip $@ $(ALL_TEST_FILES)

endif

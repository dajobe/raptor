# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor MKR tests
#
# Copyright (C) 2014, Richard H. McCullough
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

TEST_FILES=test-07.ttl test-08.ttl test-32.ttl \
rdf-schema.ttl \
rdfs-namespace.ttl \
rdfq-results.ttl

TEST_BAD_FILES=bad-00.ttl bad-01.ttl bad-02.ttl bad-03.ttl \
bad-04.ttl bad-05.ttl bad-06.ttl bad-07.ttl bad-08.ttl bad-09.ttl \
bad-10.ttl bad-11.ttl bad-12.ttl bad-13.ttl bad-14.ttl bad-16.ttl \
bad-19.ttl bad-20.ttl \
bad-23.ttl bad-24.ttl

TEST_OUT_FILES=test-07.out test-08.out test-32.out \
rdf-schema.out \
rdfs-namespace.out \
rdfq-results.out

TEST_SERIALIZE_FILES= \
test-28.ttl test-31.ttl test-32.ttl test-34.ttl

TEST_SERIALIZE_OUT_FILES= \
test-28-out.ttl test-31-out.ttl test-32-out.ttl test-34-out.ttl

TURTLE_HACK_OUT_FILES= \
ex-38-mkr.out

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

check-local: check-rdf check-bad-rdf check-mkr-serialize \
check-mkr-serialize-syntax check-mkr-parse-ntriples \
check-mkr-serialize-rdf

if MAINTAINER_MODE
check_rdf_deps = $(TEST_FILES)
endif

check-rdf: build-rapper $(check_rdf_deps)
	@result=0; \
	$(RECHO) "Testing legal mKR"; \
	for test in $(TEST_FILES); do \
	  name=`basename $$test .ttl` ; \
	  baseuri=$(BASE_URI)$$test; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i turtle -o mKR $(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status != 0 -a $$status != 2 ; then \
	    $(RECHO) "FAILED returned status $$status"; result=1; \
	    $(RECHO) $(RAPPER) -q -i mkr -o ntriples $(srcdir)/$$test $$baseuri '>' $$name.res; \
	    cat $$name.err; \
	  elif cmp $(srcdir)/$$name.out $$name.res >/dev/null 2>&1; then \
	    if test $$status = 2 ; then \
	      $(RECHO) "ok with warnings"; grep Warning $$name.err; \
	    else \
	      $(RECHO) "ok"; \
	    fi; \
	  else \
	    $(RECHO) "FAILED"; result=1; \
	    $(RECHO) $(RAPPER) -q -i mkr -o ntriples $(srcdir)/$$test $$baseuri '>' $$name.res; \
	    cat $$name.err; \
	    diff $(srcdir)/$$name.out $$name.res; \
	  fi; \
	  rm -f $$name.res $$name.err; \
	done; \
	exit $$result

if MAINTAINER_MODE
check_bad_rdf_deps = $(TEST_BAD_FILES)
endif

check-bad-rdf: build-rapper $(check_bad_rdf_deps)
	@set +e; result=0; \
	$(RECHO) "Testing bad mKR fails"; \
	for test in $(TEST_BAD_FILES); do \
	  name=`basename $$test .ttl` ; \
	  baseuri=$(BASE_URI)$$test; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i mkr -o ntriples file:$(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status -eq 1 ; then \
	    $(RECHO) "ok"; \
	  elif test $$status -eq 2 ; then \
	    $(RECHO) "FAILED - parsing succeeded with a warning";  result=1; \
	    $(RECHO) $(RAPPER) -q -i mkr -o ntriples file:$(srcdir)/$$test $$baseuri '>' $$name.res; \
	    cat $$name.res; grep Warning $$name.err; \
	  elif test $$status -eq 0 ; then \
	    $(RECHO) "FAILED - parsing succeeded but should have failed"; result=1; \
	    $(RECHO) $(RAPPER) -q -i mkr -o ntriples file:$(srcdir)/$$test $$baseuri '>' $$name.res; \
	    cat $$name.res; \
	  else \
	    $(RECHO) "FAILED - parsing failed with unknown status $$status"; result=1; \
	    $(RECHO) $(RAPPER) -q -i mkr -o ntriples file:$(srcdir)/$$test $$baseuri '>' $$name.res; \
	    cat $$name.err; \
	    cat $$name.res; \
	  fi; \
	  rm -f $$name.res $$name.err ; \
	done; \
	set -e; exit $$result

if MAINTAINER_MODE
check_mkr_serialize_deps = $(TEST_FILES)
endif

check-mkr-serialize: build-rdfdiff build-rapper $(check_mkr_serialize_deps)
	@set +e; result=0; \
	$(RECHO) "Testing mkr serialization with legal mkr"; \
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
	  $(RAPPER) -q -i mkr -o mkr $(srcdir)/$$test $$baseuri > $$name-mkr.ttl 2> $$name.err; \
	  status1=$$?; \
	  $(RDFDIFF) -f mkr -u $$baseuri -t mkr $(srcdir)/$$test $$name-mkr.ttl > $$name.res 2> $$name.err; \
	  status2=$$?; \
	  if test $$status1 = 0 -a $$status2 = 0; then \
	    $(RECHO) "ok"; \
	  else \
	    $(RECHO) "FAILED"; result=1; \
	    $(RECHO) $(RAPPER) -q -i mkr -o mkr $(srcdir)/$$test $$baseuri '>' $$name-mkr.ttl; \
	    $(RECHO) $(RDFDIFF) -f mkr -u $$baseuri -t mkr $(srcdir)/$$test $$name-mkr.ttl '>' $$name.res; \
	    cat $$name-mkr.ttl; cat $$name.err; \
	  fi; \
	  rm -f $$name-mkr.ttl $$name.res $$name.err; \
	done; \
	set -e; exit $$result

if MAINTAINER_MODE
check_mkr_serialize_syntax_deps = $(TEST_SERIALIZE_FILES)
endif

check-mkr-serialize-syntax: build-rapper $(check_mkr_serialize_syntax_deps)
	@set +e; result=0; \
	$(RECHO) "Testing mkr exact serialization output"; \
	for test in $(TEST_SERIALIZE_FILES); do \
	  name=`basename $$test .ttl` ; \
	  if test $$name = rdf-schema; then \
	    baseuri=$(RDF_NS_URI); \
	  elif test $$name = rdfs-namespace; then \
	    baseuri=$(RDFS_NS_URI); \
	  elif test $$name = test-31 -o $$name = test-32; then \
	    baseuri=-; \
	  else \
	    baseuri=$(BASE_URI)$$test; \
	  fi; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i mkr -o mkr $(srcdir)/$$test $$baseuri > $$name-mkr.ttl 2> $$name.err; \
	  cmp -s $(srcdir)/$$name-out.ttl $$name-mkr.ttl; \
	  status=$$?; \
	  if test $$status = 0; then \
	    $(RECHO) "ok"; \
	  else \
	    $(RECHO) "FAILED"; result=1; \
	    $(RECHO) $(RAPPER) -q -i mkr -o mkr $(srcdir)/$$test $$baseuri '>' $$name-mkr.ttl; \
	    cat $$name.err; \
	    diff -a -u $(srcdir)/$$name-out.ttl $$name-mkr.ttl; \
	  fi; \
	  rm -f $$name-mkr.ttl $$name.err; \
	done; \
	set -e; exit $$result

check-mkr-parse-ntriples: build-rapper
	@set +e; result=0; \
	$(RECHO) "Testing mKR parsing with N-Triples tests"; \
	NT_TEST_FILES=`unset MAKELEVEL MAKEFLAGS; cd $(NTRIPLES_DIR) && $(MAKE) print-nt-test-files | sed -e 's,^, ,' -e 's, ,$(NTRIPLES_DIR)/,g'`; \
	for test in $$NT_TEST_FILES; do \
	  name=`basename $$test .nt` ; \
	  baseuri=-; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i mkr -o ntriples file:$(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
	  test_expected=$(srcdir)/$(NTRIPLES_DIR)/$$name.out; \
	  if cmp $$test_expected $$name.res >/dev/null 2>&1; then \
	    $(RECHO) "ok"; \
	  else \
	    $(RECHO) "FAILED";  result=1; \
	    $(RECHO) $(RAPPER) -q -i mkr -o ntriples file:$(srcdir)/$$test $$baseuri '>' $$name.res; \
	    cat $$name.err; \
	    diff $$test_expected $$name.res; \
	  fi; \
	  rm -f $$name.res $$name.err; \
	done; \
	set -e; exit $$result

check-mkr-serialize-rdf: build-rdfdiff build-rapper
	@set +e; result=0; \
	RDF_TEST_FILES=`unset MAKELEVEL MAKEFLAGS; cd $(RDFXML_DIR) && $(MAKE) print-rdf-test-files | sed -e 's,^, ,' -e 's, ,$(RDFXML_DIR)/,g'`; \
	$(RECHO) "Testing mkr serialization with legal rdf/xml"; \
	for test in $$RDF_TEST_FILES $(TURTLE_SERIALIZE_RDF_FILES); do \
	  name=`basename $$test .rdf` ; \
	  baseuri=-; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -o mkr $(srcdir)/$$test $$baseuri > $$name-turtle.ttl 2> $$name.err; \
	  status1=$$?; \
	  if test $$test = "$(RDFXML_DIR)/ex-38.rdf"; then \
	    diff $(srcdir)/ex-38-mkr.out $$name-mkr.ttl > $$name.res 2> $$name.err; \
	    status2=$$?; \
	  else \
	    $(RDFDIFF) -t mkr $(srcdir)/$$test $$name-mkr.ttl > $$name.res 2> $$name.err; \
	    status2=$$?; \
	  fi; \
	  if test $$status1 = 0 -a $$status2 = 0; then \
	    $(RECHO) "ok"; \
	  else \
	    $(RECHO) "FAILED"; result=1; \
	    $(RECHO) $(RAPPER) -q -o mkr $(srcdir)/$$test $$baseuri '>' $$name-mkr.ttl; \
	    $(RECHO) $(RDFDIFF) -t mkr $(srcdir)/$$test $$name-mkr.ttl '>' $$name.res; \
	    cat $$name.err; \
	  fi; \
	  rm -f $$name-mkr.ttl $$name.res $$name.err; \
	done; \
	set -e; exit $$result


if MAINTAINER_MODE
zip: tests.zip

tests.zip: $(ALL_TEST_FILES)
	rm -f $@
	zip $@ $(ALL_TEST_FILES)

endif

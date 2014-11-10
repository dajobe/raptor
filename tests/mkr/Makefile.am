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


TEST_SERIALIZE_FILES= \
rdf-schema.ttl \
rdfs-namespace.ttl \
rdfq-results.ttl \
test-07.ttl test-08.ttl test-16a.ttl \
test-28.ttl test-31.ttl test-32.ttl test-34.ttl

TEST_SERIALIZE_OUT_FILES= \
rdf-schema.mkr \
rdfs-namespace.mkr \
rdfq-results.mkr \
test-07.mkr test-08.mkr test-16a.mkr \
test-28.mkr test-31.mkr test-32.mkr test-34.mkr

# Used to make N-triples output consistent
BASE_URI=http://www.w3.org/2001/sw/DataAccess/df1/tests/
# for rdf-schema.ttl
RDF_NS_URI=http://www.w3.org/1999/02/22-rdf-syntax-ns
# for rdfs-namespace.ttl
RDFS_NS_URI=http://www.w3.org/2000/01/rdf-schema

ALL_TEST_FILES= README.txt \
	$(TEST_SERIALIZE_FILES) \
	$(TEST_SERIALIZE_OUT_FILES) 

EXTRA_DIST = $(ALL_TEST_FILES)

RAPPER  = $(top_builddir)/utils/rapper
RDFDIFF = $(top_builddir)/utils/rdfdiff

build-rapper:
	@(cd $(top_builddir)/utils ; $(MAKE) rapper$(EXEEXT))


check-local: \
check-mkr-serialize-syntax


if MAINTAINER_MODE
check_mkr_serialize_deps = $(TEST_FILES)
endif


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
	  else \
            baseuri=$(BASE_URI)$$test; \
	  fi; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i turtle -o mkr $(srcdir)/$$test $$baseuri > $$name.out 2> $$name.err; \
	  cmp -s $$name.out $(srcdir)/$$name.mkr; \
	  status=$$?; \
	  if test $$status = 0; then \
	    $(RECHO) "ok"; \
	  else \
	    $(RECHO) "FAILED"; result=1; \
	    $(RECHO) $(RAPPER) -q -i turtle -o mkr $(srcdir)/$$test $$baseuri '>' $$name.out; \
	    cat $$name.err; \
	    diff -a -u $$name.out $(srcdir)/$$name.mkr; \
	  fi; \
	  rm -f $$name.out $$name.err; \
	done; \
	set -e; exit $$result



if MAINTAINER_MODE
zip: tests.zip

tests.zip: $(ALL_TEST_FILES)
	rm -f $@
	zip $@ $(ALL_TEST_FILES)

endif

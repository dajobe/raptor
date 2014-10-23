#
# Copyright (C) 2014, Richard H. McCullough
# 

RAPPER  = rapper
RECHO   = echo
RECHO_N = -n
RECHO_C =

TEST_SERIALIZE_FILES = \
rdf-schema.ttl \
rdfs-namespace.ttl \
rdfq-results.ttl \
test-07.ttl test-08.ttl test-16a.ttl \
test-28.ttl test-31.ttl test-32.ttl test-34.ttl

TEST_SERIALIZE_OUT_FILES = \
rdf-schema.mkr \
rdfs-namespace.mkr \
rdfq-results.mkr \
test-07.mkr test-08.mkr test-16a.mkr \
test-28.mkr test-31.mkr test-32.mkr test-34.mkr

# Used to make N-triples output consistent
BASE_URI = http://www.w3.org/2001/sw/DataAccess/df1/tests/
# for rdf-schema.ttl
RDF_NS_URI = http://www.w3.org/1999/02/22-rdf-syntax-ns
# for rdfs-namespace.ttl
RDFS_NS_URI = http://www.w3.org/2000/01/rdf-schema

ALL_TEST_FILES = README.txt \
	$(TEST_SERIALIZE_FILES) \
	$(TEST_SERIALIZE_OUT_FILES) 

EXTRA_DIST = $(ALL_TEST_FILES)



check-mkr-serialize-syntax:
	@set +e; result=0; \
	$(RECHO) "Testing mkr exact serialization output"; \
	for test in $(TEST_SERIALIZE_FILES); do \
	  name=`basename $$test .ttl` ; \
	  if test $$name = rdf-schema; then \
	    baseuri=$(RDF_NS_URI); \
	  elif test $$name = rdfs-namespace; then \
	    baseuri=$(RDFS_NS_URI); \
	  else \
	    baseuri=-; \
	  fi; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i turtle -o mkr $$test $$baseuri > $$name.out 2> $$name.err; \
	  cmp -s $$name.mkr $$name.out; \
	  status=$$?; \
	  if test $$status = 0; then \
	    $(RECHO) "ok"; \
	  else \
	    $(RECHO) "FAILED"; result=1; \
	    cat $$name.err; \
	    diff -a -u $$name.out $$name.mkr; \
	  fi; \
	  rm -f $$name.out $$name.err; \
	done; \
	set -e; exit $$result

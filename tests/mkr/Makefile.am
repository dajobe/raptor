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
base.ttl \
rdfq-results.ttl \
rdf-schema.ttl \
rdfs-namespace.ttl \
test-07.ttl \
test-07a.ttl \
test-07b.ttl \
test-07c.ttl \
test-08.ttl \
test-16a.ttl \
test-16b.ttl \
test-16c.ttl \
test-16d.ttl \
test-16e.ttl \
test-16f.ttl \
test-16g.ttl \
test-28.ttl \
test-31.ttl \
test-32.ttl \
test-34.ttl \
turtle-syntax-number-08.ttl

TEST_SERIALIZE_OUT_FILES= \
base.mkr \
rdfq-results.mkr \
rdf-schema.mkr \
rdfs-namespace.mkr \
test-07.mkr \
test-07a.mkr \
test-07b.mkr \
test-07c.mkr \
test-08.mkr \
test-16a.mkr \
test-16b.mkr \
test-16c.mkr \
test-16d.mkr \
test-16e.mkr \
test-16f.mkr \
test-16g.mkr \
test-28.mkr \
test-31.mkr \
test-32.mkr \
test-34.mkr \
turtle-syntax-number-08.ttl

TEST_PARSER_FILES= \
alias.mkr \
assignment.mkr \
base2.mkr \
command.mkr \
definition.mkr \
hierarchy.mkr \
hoVerb.mkr \
man.mkr \
question.mkr \
rdfq-results.mkr \
rdf-schema.mkr \
rdfs-namespace.mkr \
relation.mkr \
test-07.mkr \
test-07a.mkr \
test-07b.mkr \
test-07c.mkr \
test-08.mkr \
test-16a.mkr \
test-16b.mkr \
test-16c.mkr \
test-16d.mkr \
test-16e.mkr \
test-16f.mkr \
test-16g.mkr \
test-31a.mkr \
test-32.mkr \
test-34.mkr \
turtle-syntax-number-08.mkr

TEST_PARSER_OUT_FILES= \
alias.nt \
assignment.nt \
base2.nt \
command.nt \
definition.nt \
hierarchy.nt \
hoVerb.nt \
man.nt \
question.nt \
rdfq-results.nt \
rdf-schema.nt \
rdfs-namespace.nt \
relation.nt \
test-07.nt \
test-07a.nt \
test-07b.nt \
test-07c.nt \
test-08.nt \
test-16a.nt \
test-16b.nt \
test-16c.nt \
test-16d.nt \
test-16e.nt \
test-16f.nt \
test-16g.nt \
test-31a.nt \
test-32.nt \
test-34.nt \
turtle-syntax-number-08.nt

TEST_ROUNDTRIP_FILES = $(TEST_PARSER_FILES)

TEST_ROUNDTRIP_OUT_FILES= \
alias.mkr.mkr \
assignment.mkr.mkr \
base2.mkr.mkr \
command.mkr.mkr \
definition.mkr.mkr \
hierarchy.mkr.mkr \
hoVerb.mkr.mkr \
man.mkr.mkr \
question.mkr.mkr \
rdfq-results.mkr.mkr \
rdf-schema.mkr.mkr \
rdfs-namespace.mkr.mkr \
relation.mkr.mkr \
test-07.mkr.mkr \
test-07a.mkr.mkr \
test-07b.mkr.mkr \
test-07c.mkr.mkr \
test-08.mkr.mkr \
test-16a.mkr.mkr \
test-16b.mkr.mkr \
test-16c.mkr.mkr \
test-16d.mkr.mkr \
test-16e.mkr.mkr \
test-16f.mkr.mkr \
test-16g.mkr.mkr \
test-31a.mkr.mkr \
test-32.mkr.mkr \
test-34.mkr.mkr \
turtle-syntax-number-08.mkr.mkr

# Used to make N-triples output consistent
BASE_URI=http://www.w3.org/2001/sw/DataAccess/df1/tests/
# for rdf-schema.ttl
RDF_NS_URI=http://www.w3.org/1999/02/22-rdf-syntax-ns
# for rdfs-namespace.ttl
RDFS_NS_URI=http://www.w3.org/2000/01/rdf-schema
# for mKR serializer
MKR_BASE_URI=http://mkrmke.net/tests/mkr/
# for mKR parser

ALL_TEST_FILES= README.txt \
	$(TEST_SERIALIZE_FILES) \
	$(TEST_SERIALIZE_OUT_FILES) \
	$(TEST_PARSER_FILES) \
	$(TEST_PARSER_OUT_FILES) \
	$(TEST_ROUNDTRIP_OUT_FILES)

EXTRA_DIST = $(ALL_TEST_FILES)

RAPPER  = $(top_builddir)/utils/rapper
RDFDIFF = $(top_builddir)/utils/rdfdiff

build-rapper:
	@(cd $(top_builddir)/utils ; $(MAKE) rapper$(EXEEXT))

build-rdfdiff:
	@(cd $(top_builddir)/utils ; $(MAKE) rdfdiff$(EXEEXT))


check-local: \
check-mkr-serialize-syntax \
check-mkr-parser-syntax \
check-mkr-roundtrip


if MAINTAINER_MODE
check_mkr_serialize_syntax_deps = $(TEST_SERIALIZE_FILES) $(TEST_SERIALIZE_OUT_FILES)
check_mkr_parser_syntax_deps = $(TEST_PARSER_FILES) $(TEST_PARSER_OUT_FILES)
check_mkr_roundtrip_deps = $(TEST_ROUNDTRIP_FILES) $(TEST_ROUNDTRIP_OUT_FILES)
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


check-mkr-parser-syntax: build-rapper $(check_mkr_parser_syntax_deps)
	@set +e; result=0; \
	$(RECHO) "Testing mkr exact parser output"; \
	for test in $(TEST_PARSER_FILES); do \
	  name=`basename $$test .mkr` ; \
	  if test $$name = rdf-schema; then \
	    baseuri=$(RDF_NS_URI); \
	  elif test $$name = rdfs-namespace; then \
	    baseuri=$(RDFS_NS_URI); \
	  else \
            baseuri=$(BASE_URI)$$test; \
	  fi; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i mkr -o ntriples $(srcdir)/$$test $$baseuri > $$name.out 2> $$name.err; \
	  cmp -s $$name.out $(srcdir)/$$name.nt; \
	  status=$$?; \
	  if test $$status = 0; then \
	    $(RECHO) "ok"; \
	  else \
	    $(RECHO) "FAILED"; result=1; \
	    $(RECHO) $(RAPPER) -q -i mkr -o ntriples $(srcdir)/$$test $$baseuri '>' $$name.out; \
	    cat $$name.err; \
	    diff -a -u $$name.out $(srcdir)/$$name.nt; \
	  fi; \
	  rm -f $$name.out $$name.err; \
	done; \
	set -e; exit $$result


check-mkr-roundtrip: build-rapper $(check_mkr_roundtrip_deps)
	@set +e; result=0; \
	$(RECHO) "Testing mkr roundtrip output"; \
	for test in $(TEST_ROUNDTRIP_FILES); do \
	  name=`basename $$test .mkr` ; \
	  if test $$name = rdf-schema; then \
	    baseuri=$(RDF_NS_URI); \
	  elif test $$name = rdfs-namespace; then \
	    baseuri=$(RDFS_NS_URI); \
	  else \
            baseuri=$(BASE_URI)$$test; \
	  fi; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i mkr -o mkr $(srcdir)/$$test $$baseuri > $$name.out 2> $$name.err; \
	  cmp -s $$name.out $(srcdir)/$$name.mkr.mkr; \
	  status=$$?; \
	  if test $$status = 0; then \
	    $(RECHO) "ok"; \
	  else \
	    $(RECHO) "FAILED"; result=1; \
	    $(RECHO) $(RAPPER) -q -i mkr -o mkr $(srcdir)/$$test $$baseuri '>' $$name.out; \
	    cat $$name.err; \
	    diff -a -u $$name.out $(srcdir)/$$name.mkr.mkr; \
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

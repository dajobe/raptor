# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor Feed tests
#
# Copyright (C) 2009, David Beckett http://www.dajobe.org/
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

# Input RDF/XML (atom model) files - rdfxml parser
TEST_IN_RDF_ATOMS= test01.rdf test02.rdf test03.rdf
# Input Atom 1.0 (atom model) files - rss-tag-soup parser
TEST_IN_ATOMS= test04.atom test05.atom

# Output files in Turtle (after parsing) and Atom (after serializing) 
OUT_RDF_TTLS= $(TEST_IN_RDF_ATOMS:.rdf=.ttl)
OUT_ATOM_TTLS= $(TEST_IN_ATOMS:.atom=.ttl)
OUT_RDF_ATOMS= $(TEST_IN_RDF_ATOMS:.rdf=.atom)

# Expected results for above
EXPECTED_TTLS= $(OUT_RDF_TTLS:.ttl=-result.ttl) $(OUT_ATOM_TTLS:.ttl=-result.ttl)
EXPECTED_ATOMS= $(OUT_RDF_ATOMS:.atom=-result.atom)

# Files generated during testing (to delete/clean)
OUT_TTLS = $(OUT_RDF_TTLS) $(OUT_ATOM_TTLS)
OUT_ATOMS = $(OUT_RDF_ATOMS)

EXTRA_DIST = \
$(TEST_IN_RDF_ATOMS) $(TEST_IN_ATOMS) \
$(EXPECTED_TTLS) $(EXPECTED_ATOMS) \
atom.rng atom.rnc

CLEANFILES = $(OUT_ATOMS) $(OUT_TTLS) errors.log

RAPPER = $(top_builddir)/utils/rapper

build-rapper:
	@(cd $(top_builddir)/utils ; $(MAKE) rapper$(EXEEXT))

FEED_TESTS=

if RAPTOR_PARSER_RDFXML
FEED_TESTS += check-rdfxml-to-turtle
endif

if RAPTOR_PARSER_RSS
FEED_TESTS += check-atom-to-turtle
endif

if RAPTOR_SERIALIZER_ATOM
FEED_TESTS += check-serialize-atom
endif

if MAINTAINER_MODE
FEED_TESTS += check-validate-atom
endif

check-local: build-rapper $(FEED_TESTS)

# Parse from RDF/XML and Serialize to Turtle
check-rdfxml-to-turtle: $(TEST_IN_RDF_ATOMS)
	@set +e; result=0; \
	$(ECHO) "Testing RDF/XML to Turtle"; \
	for test in $?; do \
	  parser=rdfxml; \
	  name=`basename $$test .rdf` ; \
	  turtle="$$name.ttl"; \
	  expected="$$name-result.ttl"; \
	  opts="-f writeBaseURI=0"; \
	  if test $$name = test01; then \
	    baseuri="http://example.org/whatsnew"; \
	  elif test $$name = test02; then \
	    baseuri="http://example.org/news/"; \
	  elif test $$name = test03; then \
	    baseuri="http://example.org"; \
	  fi; \
	  opts="-q -i $$parser -o turtle $$opts -O $$baseuri"; \
	  $(ECHO) $(ECHO_N) "Checking $$test $(ECHO_C)"; \
	  $(RAPPER) $$opts file:$(srcdir)/$$test > $$turtle 2> errors.log; \
	  status=$$?; \
	  if test $$status != 0; then \
	    $(ECHO) "FAILED with code $$status"; \
	    $(ECHO) "$(RAPPER) $$opts file:$(srcdir)/$$test"; \
	    cat errors.log ; \
	    result=1 ; \
	  elif cmp $(srcdir)/$$expected $$turtle >/dev/null 2>&1; then \
	    $(ECHO) "ok"; \
	  else \
	    $(ECHO) "FAILED"; \
	    $(ECHO) "$(RAPPER) $$opts file:$(srcdir)/$$test"; \
	    diff -u $(srcdir)/$$expected $$turtle; result=1; \
	  fi; \
	  rm -f errors.log ; \
	done; \
	set -e; exit $$result

# Parser from Atom and Serialize to Turtle
check-atom-to-turtle: $(TEST_IN_ATOMS)
	@set +e; result=0; \
	$(ECHO) "Testing Atom to Turtle"; \
	for test in $?; do \
	  parser=rss-tag-soup; \
	  name=`basename $$test .atom` ; \
	  turtle="$$name.ttl"; \
	  expected="$$name-result.ttl"; \
	  opts="-f writeBaseURI=0"; \
	  baseuri="http://www.example.org/blog/"; \
	  if test $$name = test04; then \
	    baseuri="http://www.example.org/blog/"; \
	  fi; \
	  opts="-q -i $$parser -o turtle $$opts -O $$baseuri"; \
	  $(ECHO) $(ECHO_N) "Checking $$test $(ECHO_C)"; \
	  $(RAPPER) $$opts file:$(srcdir)/$$test > $$turtle 2> errors.log; \
	  status=$$?; \
	  if test $$status != 0; then \
	    $(ECHO) "FAILED with code $$status"; \
	    $(ECHO) "$(RAPPER) $$opts file:$(srcdir)/$$test"; \
	    cat errors.log ; \
	    result=1 ; \
	  elif cmp $(srcdir)/$$expected $$turtle >/dev/null 2>&1; then \
	    $(ECHO) "ok"; \
	  else \
	    $(ECHO) "FAILED"; \
	    $(ECHO) "$(RAPPER) $$opts file:$(srcdir)/$$test"; \
	    diff -u $(srcdir)/$$expected $$turtle; result=1; \
	  fi; \
	  rm -f errors.log ; \
	done; \
	set -e; exit $$result

# Parser from Turtle and Serialize to Atom
check-serialize-atom: $(OUT_RDF_TTLS)
	@set +e; result=0; \
	$(ECHO) "Testing Turtle to Atom XML"; \
	for test in $(OUT_RDF_TTLS); do \
	  name=`basename $$test .ttl` ; \
	  atom="$$name.atom"; \
	  expected="$$name-result.atom"; \
	  opts="-f writeBaseURI=0 -f rssTriples=atom-triples"; \
	  if test $$name = test01; then \
	    baseuri="http://example.org/whatsnew"; \
	    opts="$$opts -f atomEntryUri=http://example.org/2006/03/28/blog-item"; \
	  elif test $$name = test02; then \
	    baseuri="http://example.org/news/"; \
	  elif test $$name = test03; then \
	    baseuri="http://example.org"; \
	  fi; \
	  opts="-q -i turtle -o atom $$opts -I $$baseuri -O $$baseuri"; \
	  $(ECHO) $(ECHO_N) "Checking $$test $(ECHO_C)"; \
	  $(RAPPER) $$opts file:$$test > $$atom 2> errors.log; \
	  status=$$?; \
	  if test $$status != 0; then \
	    $(ECHO) "FAILED with code $$status"; \
	    $(ECHO) "$(RAPPER) $$opts file:$$test"; \
	    cat errors.log ; \
	    result=1 ; \
	  elif cmp $(srcdir)/$$expected $$atom >/dev/null 2>&1; then \
	    $(ECHO) "ok"; \
	  else \
	    $(ECHO) "FAILED"; \
	    $(ECHO) "$(RAPPER) $$opts file:$(srcdir)/$$test"; \
	    diff -u $(srcdir)/$$expected $$atom; result=1; \
	  fi; \
	  rm -f errors.log ; \
	done; \
	set -e; exit $$result


if MAINTAINER_MODE
SUFFIXES = .rng .rnc

.rnc.rng:
	trang $? $@

check-validate-atom: $(srcdir)/atom.rng $(OUT_ATOMS) $(TEST_IN_ATOMS)
	@set +e; result=0; \
	$(ECHO) "Validating Atom XML with RelaxNG"; \
	for atom in $(OUT_ATOMS) $(TEST_IN_ATOMS); do \
	  $(ECHO) $(ECHO_N) "Validating $$atom $(ECHO_C)"; \
	  jing $(srcdir)/atom.rng $$atom > errors.log 2>&1; \
	  status=$$?; \
	  if test $$status != 0; then \
	    $(ECHO) "FAILED with code $$status"; result=1; \
	    $(ECHO) jing $(srcdir)/atom.rng $$atom; \
	    sed -e "s,^.*$$atom,$$atom," errors.log ; \
	    result=1 ; \
	  else \
	    $(ECHO) "ok"; \
          fi; \
          rm -f errors.log; \
	done; \
	set -e; exit $$result

endif

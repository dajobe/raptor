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

# Input RSS 1.0 files
TEST_RSS=test01.rss test02.rss test03.rss
# Output files in Turtle (after parsing) and Atom (after serializing) 
TEST_TTLS=$(TEST_RSS:.rss=.ttl)
TEST_ATOMS=$(TEST_RSS:.rss=.atom)

# Expected results for above
OUT_TTLS=$(TEST_RSS:.rss=-result.ttl)
OUT_ATOMS=$(TEST_RSS:.rss=-result.atom)

ALL_TEST_FILES= \
	$(TEST_ATOMS)

EXTRA_DIST = $(TEST_RSS) $(OUT_ATOMS) atom.rng atom.rnc

CLEANFILES = $(TEST_ATOMS) $(TEST_TTLS) errors.log

RAPPER = $(top_builddir)/utils/rapper

build-rapper:
	@(cd $(top_builddir)/utils ; $(MAKE) rapper$(EXEEXT))

if MAINTAINER_MODE
check-local:: check-feeds check-validate-atom

SUFFIXES = .atom .rss .rng .rnc

.rnc.rng:
	trang $? $@


check-validate-atom: $(srcdir)/atom.rng $(TEST_ATOMS)
	@echo "Validating Atom XML with RelaxNG"; \
	for atom in $(TEST_ATOMS); do \
	  echo "$$atom"; \
	  jing $(srcdir)/atom.rng $$atom; \
	done
# or xmllint --relaxng atom.rng --noout $$atom

else

check-local:: check-feeds

endif


@RAPTOR_PARSER_RSS_TRUE@check-local:: check-parse-feed
@RAPTOR_SERIALIZER_ATOM_TRUE@check-local:: check-serialize-atom

check-feeds: build-rapper $(TEST_FILES)

check-parse-feed $(TEST_TTLS): $(TEST_RSS)
	@set +e; result=0; \
	echo "Testing Feed parsing"; \
	for test in $(TEST_RSS); do \
	  name=`basename $$test .rss` ; \
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
	  opts="-q -i rdfxml -o turtle $$opts -O $$baseuri"; \
	  echo $(ECHO_N) "Checking $$test $(ECHO_C)"; \
	  $(RAPPER) $$opts file:$(srcdir)/$$test > $$turtle 2> errors.log; \
	  status=$$?; \
	  if test $$status != 0; then \
	    echo "FAILED with code $$status"; \
	    echo "$(RAPPER) $$opts file:$(srcdir)/$$test"; \
	    cat errors.log ; \
	  elif cmp $(srcdir)/$$expected $$turtle >/dev/null 2>&1; then \
	    echo "ok"; \
	  else \
	    echo "FAILED"; \
	    echo "$(RAPPER) $$opts file:$(srcdir)/$$test"; \
	    diff -u $(srcdir)/$$expected $$turtle; result=1; \
	  fi; \
	  rm -f errors.log ; \
	done; \
	set -e; exit $$result

check-serialize-atom $(TEST_ATOMS): $(TEST_TTLS)
	@set +e; result=0; \
	echo "Testing Atom XML serializing"; \
	for test in $(TEST_TTLS); do \
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
	  opts="-q -i turtle -o atom $$opts -O $$baseuri"; \
	  echo $(ECHO_N) "Checking $$test $(ECHO_C)"; \
	  $(RAPPER) $$opts file:$(srcdir)/$$test > $$atom 2> errors.log; \
	  status=$$?; \
	  if test $$status != 0; then \
	    echo "FAILED with code $$status"; \
	    echo "$(RAPPER) $$opts file:$(srcdir)/$$test"; \
	    cat errors.log ; \
	  elif cmp $(srcdir)/$$expected $$atom >/dev/null 2>&1; then \
	    echo "ok"; \
	  else \
	    echo "FAILED"; \
	    echo "$(RAPPER) $$opts file:$(srcdir)/$$test"; \
	    diff -u $(srcdir)/$$expected $$atom; result=1; \
	  fi; \
	  rm -f errors.log ; \
	done; \
	set -e; exit $$result

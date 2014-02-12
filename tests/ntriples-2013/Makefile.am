# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor N-Triples 2013 tests
#
# Copyright (C) 2013, David Beckett http://www.dajobe.org/
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

RDFCOMPARE=$(srcdir)/../../scripts/rdfcompare

RDFXML_DIR = ../rdfxml

TEST_GOOD_FILES = \
  nt-syntax-file-01.nt \
  nt-syntax-file-02.nt \
  nt-syntax-file-03.nt \
  nt-syntax-uri-01.nt \
  nt-syntax-uri-02.nt \
  nt-syntax-uri-03.nt \
  nt-syntax-uri-04.nt \
  nt-syntax-string-01.nt \
  nt-syntax-string-02.nt \
  nt-syntax-string-03.nt \
  nt-syntax-str-esc-01.nt \
  nt-syntax-str-esc-02.nt \
  nt-syntax-str-esc-03.nt \
  nt-syntax-bnode-01.nt \
  nt-syntax-bnode-02.nt \
  nt-syntax-bnode-03.nt \
  nt-syntax-datatypes-01.nt \
  nt-syntax-datatypes-02.nt \
  nt-syntax-subm-01.nt \
  comment_following_triple.nt \
  LITERAL.nt \
  LITERAL_all_controls.nt \
  LITERAL_all_punctuation.nt \
  LITERAL_ascii_boundaries.nt \
  literal_false.nt \
  literal_true.nt \
  LITERAL_with_2_dquotes.nt \
  LITERAL_with_2_squotes.nt \
  literal_with_BACKSPACE.nt \
  literal_with_CARRIAGE_RETURN.nt \
  literal_with_CHARACTER_TABULATION.nt \
  literal_with_FORM_FEED.nt \
  literal_with_LINE_FEED.nt \
  literal_with_REVERSE_SOLIDUS.nt \
  literal_with_REVERSE_SOLIDUS2.nt \
  LITERAL_with_UTF8_boundaries.nt \
  LITERAL_with_dquote.nt \
  literal_with_numeric_escape4.nt \
  literal_with_numeric_escape8.nt \
  LITERAL_with_squote.nt \
  langtagged_string.nt \
  lantag_with_subtag.nt \
  minimal_whitespace.nt

TEST_BAD_FILES = \
  nt-syntax-bad-uri-01.nt \
  nt-syntax-bad-uri-02.nt \
  nt-syntax-bad-uri-03.nt \
  nt-syntax-bad-uri-04.nt \
  nt-syntax-bad-uri-05.nt \
  nt-syntax-bad-uri-06.nt \
  nt-syntax-bad-uri-07.nt \
  nt-syntax-bad-uri-08.nt \
  nt-syntax-bad-uri-09.nt \
  nt-syntax-bad-prefix-01.nt \
  nt-syntax-bad-base-01.nt \
  nt-syntax-bad-struct-01.nt \
  nt-syntax-bad-struct-02.nt \
  nt-syntax-bad-lang-01.nt \
  nt-syntax-bad-esc-01.nt \
  nt-syntax-bad-esc-02.nt \
  nt-syntax-bad-esc-03.nt \
  nt-syntax-bad-string-01.nt \
  nt-syntax-bad-string-02.nt \
  nt-syntax-bad-string-03.nt \
  nt-syntax-bad-string-04.nt \
  nt-syntax-bad-string-05.nt \
  nt-syntax-bad-string-06.nt \
  nt-syntax-bad-string-07.nt \
  nt-syntax-bad-num-01.nt \
  nt-syntax-bad-num-02.nt \
  nt-syntax-bad-num-03.nt

# Used to make N-triples output consistent
BASE_URI=http://www.w3.org/2013/N-TriplesTests/
# for rdf-schema.ttl
RDF_NS_URI=http://www.w3.org/1999/02/22-rdf-syntax-ns
# for rdfs-namespace.ttl
RDFS_NS_URI=http://www.w3.org/2000/01/rdf-schema

ALL_TEST_FILES= \
	$(TEST_GOOD_FILES) \
	$(TEST_BAD_FILES)

EXTRA_DIST = README \
	$(ALL_TEST_FILES)

RAPPER  = $(top_builddir)/utils/rapper
RDFDIFF = $(top_builddir)/utils/rdfdiff

build-rapper:
	@(cd $(top_builddir)/utils ; $(MAKE) rapper$(EXEEXT))

build-rdfdiff:
	@(cd $(top_builddir)/utils ; $(MAKE) rdfdiff$(EXEEXT))

check-local: check-good-ntriples check-bad-ntriples

if MAINTAINER_MODE
check_good_ntriples_deps = $(TEST_GOOD_FILES)
endif

check-good-ntriples: build-rapper $(check_good_ntriples_deps)
	@set +e; result=0; errors=0; failures=''; \
	$(RECHO) "Testing good N-Triples syntax passes"; \
	for test in $(TEST_GOOD_FILES); do \
	  name=`basename $$test .ttl` ; \
	  baseuri=$(BASE_URI)$$test; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i ntriples -o ntriples file:$(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status != 0 -a $$status != 2 ; then \
	    $(RECHO) "FAILED returned status $$status"; result=1; \
	    $(RECHO) $(RAPPER) -q -i ntriples -o ntriples $(srcdir)/$$test $$baseuri '>' $$name.res; \
	    cat $$name.err; \
	    errors=`expr $$errors + 1`; \
	    failures="$$failures $$test"; \
	  elif test $$status = 2 ; then \
	    $(RECHO) "ok with warnings"; grep Warning $$name.err; \
	  else \
	    $(RECHO) "ok"; \
	  fi; \
	  rm -f $$name.res $$name.err; \
	done; \
	$(RECHO) "Result: $$errors errors:$$failures"; \
	set -e; exit $$result

if MAINTAINER_MODE
check_bad_ntriples_deps = $(TEST_BAD_FILES)
endif

check-bad-ntriples: build-rapper $(check_bad_ntriples_deps)
	@set +e; result=0; errors=0; failures=''; \
	$(RECHO) "Testing bad N-Triples fails"; \
	for test in $(TEST_BAD_FILES) ; do \
	  name=`basename $$test .ttl` ; \
	  baseuri=$(BASE_URI)$$test; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i ntriples -o ntriples file:$(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status -eq 1 ; then \
	    $(RECHO) "ok"; \
	  elif test $$status -eq 2 ; then \
	    $(RECHO) "FAILED - parsing succeeded with a warning";  result=1; \
	    $(RECHO) $(RAPPER) -q -i ntriples -o ntriples file:$(srcdir)/$$test $$baseuri '>' $$name.res; \
	    cat $$name.res; grep Warning $$name.err; \
	    errors=`expr $$errors + 1`; \
	    failures="$$failures $$test"; \
	  elif test $$status -eq 0 ; then \
	    $(RECHO) "FAILED - parsing succeeded but should have failed"; result=1; \
	    $(RECHO) $(RAPPER) -q -i ntriples -o ntriples file:$(srcdir)/$$test $$baseuri '>' $$name.res; \
	    cat $$name.res; \
	    errors=`expr $$errors + 1`; \
	    failures="$$failures $$test"; \
	  else \
	    $(RECHO) "FAILED - parsing failed with unknown status $$status"; result=1; \
	    $(RECHO) $(RAPPER) -q -i ntriples -o ntriples file:$(srcdir)/$$test $$baseuri '>' $$name.res; \
	    cat $$name.err; \
	    cat $$name.res; \
	    errors=`expr $$errors + 1`; \
	    failures="$$failures $$test"; \
	  fi; \
	  rm -f $$name.res $$name.err ; \
	done; \
	$(RECHO) "Result: $$errors errors:$$failures"; \
	set -e; exit $$result

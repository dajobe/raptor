# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor N-Quads 2013 tests
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
NTRIPLES_DIR = ../ntriples

TEST_GOOD_FILES = \
comment_following_triple.nq \
langtagged_string.nq \
lantag_with_subtag.nq \
literal_all_controls.nq \
literal_all_punctuation.nq \
literal_ascii_boundaries.nq \
literal_false.nq \
literal.nq \
literal_true.nq \
literal_with_2_dquotes.nq \
literal_with_2_squotes.nq \
literal_with_BACKSPACE.nq \
literal_with_CARRIAGE_RETURN.nq \
literal_with_CHARACTER_TABULATION.nq \
literal_with_dquote.nq \
literal_with_FORM_FEED.nq \
literal_with_LINE_FEED.nq \
literal_with_numeric_escape4.nq \
literal_with_numeric_escape8.nq \
literal_with_REVERSE_SOLIDUS2.nq \
literal_with_REVERSE_SOLIDUS.nq \
literal_with_squote.nq \
literal_with_UTF8_boundaries.nq \
minimal_whitespace.nq \
nq-syntax-bnode-01.nq \
nq-syntax-bnode-02.nq \
nq-syntax-bnode-03.nq \
nq-syntax-bnode-04.nq \
nq-syntax-bnode-05.nq \
nq-syntax-bnode-06.nq \
nq-syntax-uri-01.nq \
nq-syntax-uri-02.nq \
nq-syntax-uri-03.nq \
nq-syntax-uri-04.nq \
nq-syntax-uri-05.nq \
nq-syntax-uri-06.nq \
nt-syntax-bnode-01.nq \
nt-syntax-bnode-02.nq \
nt-syntax-bnode-03.nq \
nt-syntax-datatypes-01.nq \
nt-syntax-datatypes-02.nq \
nt-syntax-file-01.nq \
nt-syntax-file-02.nq \
nt-syntax-file-03.nq \
nt-syntax-str-esc-01.nq \
nt-syntax-str-esc-02.nq \
nt-syntax-str-esc-03.nq \
nt-syntax-string-01.nq \
nt-syntax-string-02.nq \
nt-syntax-string-03.nq \
nt-syntax-subm-01.nq \
nt-syntax-uri-01.nq \
nt-syntax-uri-02.nq \
nt-syntax-uri-03.nq \
nt-syntax-uri-04.nq


TEST_BAD_FILES = \
nq-syntax-bad-literal-01.nq \
nq-syntax-bad-literal-02.nq \
nq-syntax-bad-literal-03.nq \
nq-syntax-bad-quint-01.nq \
nq-syntax-bad-uri-01.nq \
nt-syntax-bad-base-01.nq \
nt-syntax-bad-esc-01.nq \
nt-syntax-bad-esc-02.nq \
nt-syntax-bad-esc-03.nq \
nt-syntax-bad-lang-01.nq \
nt-syntax-bad-num-01.nq \
nt-syntax-bad-num-02.nq \
nt-syntax-bad-num-03.nq \
nt-syntax-bad-prefix-01.nq \
nt-syntax-bad-string-01.nq \
nt-syntax-bad-string-02.nq \
nt-syntax-bad-string-03.nq \
nt-syntax-bad-string-04.nq \
nt-syntax-bad-string-05.nq \
nt-syntax-bad-string-06.nq \
nt-syntax-bad-string-07.nq \
nt-syntax-bad-struct-01.nq \
nt-syntax-bad-struct-02.nq \
nt-syntax-bad-uri-01.nq \
nt-syntax-bad-uri-02.nq \
nt-syntax-bad-uri-03.nq \
nt-syntax-bad-uri-04.nq \
nt-syntax-bad-uri-05.nq \
nt-syntax-bad-uri-06.nq \
nt-syntax-bad-uri-07.nq \
nt-syntax-bad-uri-08.nq \
nt-syntax-bad-uri-09.nq

# Used to make N-triples output consistent
BASE_URI=http://www.w3.org/2013/N-QuadsTests/
# for rdf-schema.ttl
RDF_NS_URI=http://www.w3.org/1999/02/22-rdf-syntax-ns
# for rdfs-namespace.ttl
RDFS_NS_URI=http://www.w3.org/2000/01/rdf-schema

ALL_TEST_FILES= README \
	$(TEST_GOOD_FILES) \
	$(TEST_BAD_FILES)

EXTRA_DIST = $(ALL_TEST_FILES)

RAPPER  = $(top_builddir)/utils/rapper
RDFDIFF = $(top_builddir)/utils/rdfdiff

build-rapper:
	@(cd $(top_builddir)/utils ; $(MAKE) rapper$(EXEEXT))

build-rdfdiff:
	@(cd $(top_builddir)/utils ; $(MAKE) rdfdiff$(EXEEXT))

check-local: check-good-nquads check-bad-nquads

if MAINTAINER_MODE
check_good_ntriples_deps = $(TEST_GOOD_FILES)
endif

check-good-nquads: build-rapper $(check_good_nquads_deps)
	@set +e; result=0; errors=0; failures=''; \
	$(RECHO) "Testing good N-Quads syntax passes"; \
	for test in $(TEST_GOOD_FILES); do \
	  name=`basename $$test .ttl` ; \
	  baseuri=$(BASE_URI)$$test; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i nquads -o nquads file:$(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status != 0 -a $$status != 2 ; then \
	    $(RECHO) "FAILED returned status $$status"; result=1; \
	    $(RECHO) $(RAPPER) -q -i nquads -o nquads $(srcdir)/$$test $$baseuri '>' $$name.res; \
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
check_bad_nquads_deps = $(TEST_BAD_FILES)
endif

check-bad-nquads: build-rapper $(check_bad_nquads_deps)
	@set +e; result=0; errors=0; failures=''; \
	$(RECHO) "Testing bad N-Quads fails"; \
	for test in $(TEST_BAD_FILES) ; do \
	  name=`basename $$test .ttl` ; \
	  baseuri=$(BASE_URI)$$test; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i nquads -o nquads file:$(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status -eq 1 ; then \
	    $(RECHO) "ok"; \
	  elif test $$status -eq 2 ; then \
	    $(RECHO) "FAILED - parsing succeeded with a warning";  result=1; \
	    $(RECHO) $(RAPPER) -q -i nquads -o nquads file:$(srcdir)/$$test $$baseuri '>' $$name.res; \
	    cat $$name.res; grep Warning $$name.err; \
	    errors=`expr $$errors + 1`; \
	    failures="$$failures $$test"; \
	  elif test $$status -eq 0 ; then \
	    $(RECHO) "FAILED - parsing succeeded but should have failed"; result=1; \
	    $(RECHO) $(RAPPER) -q -i nquads -o nquads file:$(srcdir)/$$test $$baseuri '>' $$name.res; \
	    cat $$name.res; \
	    errors=`expr $$errors + 1`; \
	    failures="$$failures $$test"; \
	  else \
	    $(RECHO) "FAILED - parsing failed with unknown status $$status"; result=1; \
	    $(RECHO) $(RAPPER) -q -i nquads -o nquads file:$(srcdir)/$$test $$baseuri '>' $$name.res; \
	    cat $$name.err; \
	    cat $$name.res; \
	    errors=`expr $$errors + 1`; \
	    failures="$$failures $$test"; \
	  fi; \
	  rm -f $$name.res $$name.err ; \
	done; \
	$(RECHO) "Result: $$errors errors:$$failures"; \
	set -e; exit $$result

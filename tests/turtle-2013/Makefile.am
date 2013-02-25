# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor Turtle 2013 tests
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

RDFXML_DIR = ../rdfxml
NTRIPLES_DIR = ../ntriples

TEST_FILES = turtle-subm-01.ttl turtle-subm-02.ttl		\
  turtle-subm-03.ttl turtle-subm-04.ttl turtle-subm-05.ttl	\
  turtle-subm-06.ttl turtle-subm-07.ttl turtle-subm-08.ttl	\
  turtle-subm-09.ttl turtle-subm-10.ttl turtle-subm-11.ttl	\
  turtle-subm-12.ttl turtle-subm-13.ttl turtle-subm-14.ttl	\
  turtle-subm-15.ttl turtle-subm-16.ttl turtle-subm-17.ttl	\
  turtle-subm-18.ttl turtle-subm-19.ttl turtle-subm-20.ttl	\
  turtle-subm-21.ttl turtle-subm-22.ttl turtle-subm-23.ttl	\
  turtle-subm-24.ttl turtle-subm-25.ttl turtle-subm-26.ttl	\
  turtle-subm-27.ttl \
  turtle-eval-struct-01.ttl turtle-eval-struct-02.ttl

TEST_SYNTAX_FILES = turtle-syntax-base-01.ttl		\
  turtle-syntax-base-02.ttl turtle-syntax-base-03.ttl		\
  turtle-syntax-base-04.ttl turtle-syntax-bnode-01.ttl		\
  turtle-syntax-bnode-02.ttl turtle-syntax-bnode-03.ttl		\
  turtle-syntax-bnode-04.ttl turtle-syntax-bnode-05.ttl		\
  turtle-syntax-bnode-06.ttl turtle-syntax-bnode-07.ttl		\
  turtle-syntax-bnode-08.ttl turtle-syntax-bnode-09.ttl		\
  turtle-syntax-bnode-10.ttl turtle-syntax-datatypes-01.ttl	\
  turtle-syntax-datatypes-02.ttl turtle-syntax-file-01.ttl	\
  turtle-syntax-file-02.ttl turtle-syntax-file-03.ttl		\
  turtle-syntax-kw-01.ttl turtle-syntax-kw-02.ttl			\
  turtle-syntax-kw-03.ttl turtle-syntax-lists-01.ttl		\
  turtle-syntax-lists-02.ttl turtle-syntax-lists-03.ttl		\
  turtle-syntax-lists-04.ttl turtle-syntax-lists-05.ttl		\
  turtle-syntax-number-01.ttl turtle-syntax-number-02.ttl		\
  turtle-syntax-number-03.ttl turtle-syntax-number-04.ttl		\
  turtle-syntax-number-05.ttl turtle-syntax-number-06.ttl		\
  turtle-syntax-number-07.ttl turtle-syntax-number-08.ttl		\
  turtle-syntax-number-09.ttl turtle-syntax-number-10.ttl		\
  turtle-syntax-number-11.ttl turtle-syntax-pname-esc-01.ttl	\
  turtle-syntax-pname-esc-02.ttl turtle-syntax-pname-esc-03.ttl	\
  turtle-syntax-prefix-01.ttl turtle-syntax-prefix-02.ttl		\
  turtle-syntax-prefix-03.ttl turtle-syntax-prefix-04.ttl		\
  turtle-syntax-prefix-05.ttl turtle-syntax-prefix-06.ttl		\
  turtle-syntax-prefix-07.ttl turtle-syntax-prefix-08.ttl		\
  turtle-syntax-prefix-09.ttl turtle-syntax-str-esc-01.ttl	\
  turtle-syntax-str-esc-02.ttl turtle-syntax-str-esc-03.ttl	\
  turtle-syntax-string-01.ttl turtle-syntax-string-02.ttl		\
  turtle-syntax-string-03.ttl turtle-syntax-string-04.ttl		\
  turtle-syntax-string-05.ttl turtle-syntax-string-06.ttl		\
  turtle-syntax-string-07.ttl turtle-syntax-string-08.ttl		\
  turtle-syntax-string-09.ttl turtle-syntax-string-10.ttl		\
  turtle-syntax-string-11.ttl turtle-syntax-struct-01.ttl		\
  turtle-syntax-struct-02.ttl turtle-syntax-struct-03.ttl		\
  turtle-syntax-struct-04.ttl turtle-syntax-struct-05.ttl		\
  turtle-syntax-uri-01.ttl turtle-syntax-uri-02.ttl		\
  turtle-syntax-uri-03.ttl turtle-syntax-uri-04.ttl

TEST_BAD_FILES = turtle-eval-bad-01.ttl turtle-eval-bad-02.ttl		\
  turtle-eval-bad-03.ttl turtle-eval-bad-04.ttl			\
  turtle-syntax-bad-base-01.ttl turtle-syntax-bad-base-02.ttl		\
  turtle-syntax-bad-base-03.ttl turtle-syntax-bad-esc-01.ttl		\
  turtle-syntax-bad-esc-02.ttl turtle-syntax-bad-esc-03.ttl		\
  turtle-syntax-bad-esc-04.ttl turtle-syntax-bad-kw-01.ttl		\
  turtle-syntax-bad-kw-02.ttl turtle-syntax-bad-kw-03.ttl		\
  turtle-syntax-bad-kw-04.ttl turtle-syntax-bad-kw-05.ttl		\
  turtle-syntax-bad-lang-01.ttl turtle-syntax-bad-n3-extras-01.ttl	\
  turtle-syntax-bad-n3-extras-02.ttl					\
  turtle-syntax-bad-n3-extras-03.ttl					\
  turtle-syntax-bad-n3-extras-04.ttl					\
  turtle-syntax-bad-n3-extras-05.ttl					\
  turtle-syntax-bad-n3-extras-06.ttl					\
  turtle-syntax-bad-n3-extras-07.ttl					\
  turtle-syntax-bad-n3-extras-08.ttl					\
  turtle-syntax-bad-n3-extras-09.ttl					\
  turtle-syntax-bad-n3-extras-10.ttl					\
  turtle-syntax-bad-n3-extras-11.ttl					\
  turtle-syntax-bad-n3-extras-12.ttl					\
  turtle-syntax-bad-n3-extras-13.ttl turtle-syntax-bad-num-01.ttl	\
  turtle-syntax-bad-num-02.ttl turtle-syntax-bad-num-03.ttl		\
  turtle-syntax-bad-num-04.ttl turtle-syntax-bad-num-05.ttl		\
  turtle-syntax-bad-pname-01.ttl turtle-syntax-bad-pname-02.ttl	\
  turtle-syntax-bad-pname-03.ttl turtle-syntax-bad-prefix-01.ttl	\
  turtle-syntax-bad-prefix-02.ttl turtle-syntax-bad-prefix-03.ttl	\
  turtle-syntax-bad-prefix-04.ttl turtle-syntax-bad-prefix-05.ttl	\
  turtle-syntax-bad-string-01.ttl turtle-syntax-bad-string-02.ttl	\
  turtle-syntax-bad-string-03.ttl turtle-syntax-bad-string-04.ttl	\
  turtle-syntax-bad-string-05.ttl turtle-syntax-bad-string-06.ttl	\
  turtle-syntax-bad-string-07.ttl turtle-syntax-bad-struct-01.ttl	\
  turtle-syntax-bad-struct-02.ttl turtle-syntax-bad-struct-03.ttl	\
  turtle-syntax-bad-struct-04.ttl turtle-syntax-bad-struct-05.ttl	\
  turtle-syntax-bad-struct-06.ttl turtle-syntax-bad-struct-07.ttl	\
  turtle-syntax-bad-struct-08.ttl turtle-syntax-bad-struct-09.ttl	\
  turtle-syntax-bad-struct-10.ttl turtle-syntax-bad-struct-11.ttl	\
  turtle-syntax-bad-struct-12.ttl turtle-syntax-bad-struct-13.ttl	\
  turtle-syntax-bad-struct-14.ttl turtle-syntax-bad-struct-15.ttl	\
  turtle-syntax-bad-struct-16.ttl turtle-syntax-bad-struct-17.ttl	\
  turtle-syntax-bad-uri-01.ttl turtle-syntax-bad-uri-02.ttl		\
  turtle-syntax-bad-uri-03.ttl turtle-syntax-bad-uri-04.ttl		\
  turtle-syntax-bad-uri-05.ttl

TEST_OUT_FILES = turtle-subm-01.nt	\
  turtle-subm-02.nt turtle-subm-03.nt turtle-subm-04.nt	\
  turtle-subm-05.nt turtle-subm-06.nt turtle-subm-07.nt	\
  turtle-subm-08.nt turtle-subm-09.nt turtle-subm-10.nt	\
  turtle-subm-11.nt turtle-subm-12.nt turtle-subm-13.nt	\
  turtle-subm-14.nt turtle-subm-15.nt turtle-subm-16.nt	\
  turtle-subm-17.nt turtle-subm-18.nt turtle-subm-19.nt	\
  turtle-subm-20.nt turtle-subm-21.nt turtle-subm-22.nt	\
  turtle-subm-23.nt turtle-subm-24.nt turtle-subm-25.nt	\
  turtle-subm-26.nt turtle-subm-27.nt \
  turtle-eval-struct-01.nt turtle-eval-struct-02.nt

TEST_MANIFEST_FILES=manifest.ttl

# Used to make N-triples output consistent
BASE_URI=http://example/base/
# for rdf-schema.ttl
RDF_NS_URI=http://www.w3.org/1999/02/22-rdf-syntax-ns
# for rdfs-namespace.ttl
RDFS_NS_URI=http://www.w3.org/2000/01/rdf-schema

ALL_TEST_FILES= README \
	$(TEST_FILES) \
	$(TEST_SYNTAX_FILES) \
	$(TEST_BAD_FILES) \
	$(TEST_OUT_FILES) \
	$(TEST_MANIFEST_FILES)

EXTRA_DIST = $(ALL_TEST_FILES)

RAPPER  = $(top_builddir)/utils/rapper
RDFDIFF = $(top_builddir)/utils/rdfdiff

build-rapper:
	@(cd $(top_builddir)/utils ; $(MAKE) rapper$(EXEEXT))

build-rdfdiff:
	@(cd $(top_builddir)/utils ; $(MAKE) rdfdiff$(EXEEXT))

check-local: check-turtle check-bad-turtle check-turtle-syntax

if MAINTAINER_MODE
check_turtle_deps = $(TEST_FILES)
endif

check-turtle: build-rapper $(check_turtle_deps)
	@result=0; \
	$(RECHO) "Testing legal Turtle"; \
	for test in $(TEST_FILES); do \
	  name=`basename $$test .ttl` ; \
	  baseuri=$(BASE_URI)$$test; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i turtle -o ntriples $(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status != 0 -a $$status != 2 ; then \
	    $(RECHO) "FAILED returned status $$status"; result=1; \
	    $(RECHO) $(RAPPER) -q -i turtle -o ntriples $(srcdir)/$$test $$baseuri '>' $$name.res; \
	    cat $$name.err; \
	  elif cmp $(srcdir)/$$name.nt $$name.res >/dev/null 2>&1; then \
	    if test $$status = 2 ; then \
	      $(RECHO) "ok with warnings"; grep Warning $$name.err; \
	    else \
	      $(RECHO) "ok"; \
	    fi; \
	  else \
	    $(RECHO) "FAILED"; result=1; \
	    $(RECHO) $(RAPPER) -q -i turtle -o ntriples $(srcdir)/$$test $$baseuri '>' $$name.res; \
	    cat $$name.err; \
	    diff $(srcdir)/$$name.nt $$name.res; \
	  fi; \
	  rm -f $$name.res $$name.err; \
	done; \
	exit $$result

if MAINTAINER_MODE
check_bad_turtle_deps = $(TEST_BAD_FILES)
endif

check-bad-turtle: build-rapper $(check_bad_turtle_deps)
	@set +e; result=0; errors=0; failures=''; \
	$(RECHO) "Testing bad Turtle fails"; \
	for test in $(TEST_BAD_FILES); do \
	  name=`basename $$test .ttl` ; \
	  baseuri=$(BASE_URI)$$test; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i turtle -o ntriples file:$(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status -eq 1 ; then \
	    $(RECHO) "ok"; \
	  elif test $$status -eq 2 ; then \
	    $(RECHO) "FAILED - parsing succeeded with a warning";  result=1; \
	    $(RECHO) $(RAPPER) -q -i turtle -o ntriples file:$(srcdir)/$$test $$baseuri '>' $$name.res; \
	    cat $$name.res; grep Warning $$name.err; \
	    errors=`expr $$errors + 1`; \
	    failures="$$failures $$test"; \
	  elif test $$status -eq 0 ; then \
	    $(RECHO) "FAILED - parsing succeeded but should have failed"; result=1; \
	    $(RECHO) $(RAPPER) -q -i turtle -o ntriples file:$(srcdir)/$$test $$baseuri '>' $$name.res; \
	    cat $$name.res; \
	    errors=`expr $$errors + 1`; \
	    failures="$$failures $$test"; \
	  else \
	    $(RECHO) "FAILED - parsing failed with unknown status $$status"; result=1; \
	    $(RECHO) $(RAPPER) -q -i turtle -o ntriples file:$(srcdir)/$$test $$baseuri '>' $$name.res; \
	    cat $$name.err; \
	    cat $$name.res; \
	    errors=`expr $$errors + 1`; \
	    failures="$$failures $$test"; \
	  fi; \
	  rm -f $$name.res $$name.err ; \
	done; \
	$(RECHO) "Result: $$errors errors: $$failures"; \
	set -e; exit $$result


if MAINTAINER_MODE
check_turtle_syntax_deps = $(TEST_FILES) $(TEST_SYNTAX_FILES)
endif

check-turtle-syntax: build-rapper $(check_turtle_syntax_deps)
	@set +e; result=0; errors=0; failures=''; \
	$(RECHO) "Testing Turtle syntax"; \
	for test in $(TEST_FILES) $(TEST_SYNTAX_FILES); do \
	  name=`basename $$test .ttl` ; \
	  baseuri=$(BASE_URI)$$test; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i turtle -o ntriples file:$(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status != 0 -a $$status != 2 ; then \
	    $(RECHO) "FAILED returned status $$status"; result=1; \
	    $(RECHO) $(RAPPER) -q -i turtle -o ntriples $(srcdir)/$$test $$baseuri '>' $$name.res; \
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
	$(RECHO) "Result: $$errors errors: $$failures"; \
	set -e; exit $$result

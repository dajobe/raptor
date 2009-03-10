# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor Feed tests
#
# Copyright (C) 2009, David Beckett http://purl.org/net/dajobe/
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

TEST_RSS=test01.rss test02.rss test03.rss
TEST_ATOMS=$(TEST_RSS:.rss=.atom)
EXTRA_ATOMS=

ALL_TEST_FILES= \
	$(TEST_ATOMS)

EXTRA_DIST = $(ALL_TEST_FILES) atom.rng atom.rnc

CLEANFILES = $(TEST_ATOMS) errors.log

build-rapper:
	@(cd $(top_builddir)/utils ; $(MAKE) rapper$(EXEEXT))

check-local: check-feeds
@RAPTOR_SERIALIZER_ATOM_TRUE@check-local: check-feeds check-serialize-atom

if MAINTAINER_MODE
SUFFIXES = .atom .rss .rng .rnc

.rnc.rng:
	trang $? $@
endif

check-feeds: build-rapper $(TEST_FILES)

check-serialize-atom: $(TEST_ATOMS) check-validate-atom

check-validate-atom: $(srcdir)/atom.rng $(TEST_ATOMS)
	@echo "Validating Atom XML with RelaxNG"; \
	for atom in $(TEST_ATOMS); do \
	  echo "$$atom"; \
	  jing $(srcdir)/atom.rng $$atom; \
	done
# or xmllint --relaxng atom.rng --noout $$atom


test01.atom: $(srcdir)/test01.rss $(top_builddir)/utils/rapper
	$(top_builddir)/utils/rapper -q -i rdfxml -o atom -O http://example.org/whatsnew -f writeBaseURI=0 -f rssTriples=atom-triples -f atomEntryUri=http://example.org/2006/03/28/blog-item $< > $@ 2> errors.log || (cat errors.log && exit 1)

test02.atom: $(srcdir)/test02.rss $(top_builddir)/utils/rapper
	$(top_builddir)/utils/rapper -q -i rdfxml -o atom -O http://example.org/news/ -f writeBaseURI=0 -f rssTriples=atom-triples $< > $@ 2> errors.log || (cat errors.log && exit 1)

test03.atom: $(srcdir)/test03.rss $(top_builddir)/utils/rapper
	$(top_builddir)/utils/rapper -q -i rdfxml -o atom -O http://example.org -f writeBaseURI=0 -f rssTriples=atom-triples $< > $@ 2> errors.log || (cat errors.log && exit 1)

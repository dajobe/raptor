# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor GRDDL tests
#
# Copyright (C) 2007, David Beckett http://purl.org/net/dajobe/
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

TEST_FILES=test-01.html
TEST_BAD_FILES=
TEST_OUT_FILES=test-01.out
TEST_DATA_FILES=\
data-01.rdf data-02.rdf data-01.nt

ALL_TEST_FILES= \
	$(TEST_FILES) \
	$(TEST_BAD_FILES) \
	$(TEST_OUT_FILES) \
	$(TEST_DATA_FILES)

EXTRA_DIST = CMakeLists.txt $(ALL_TEST_FILES)

CLEANFILES = CMakeTests.txt

RAPPER = $(top_builddir)/utils/rapper

build-rapper:
	@(cd $(top_builddir)/utils ; $(MAKE) rapper$(EXEEXT))

if RAPTOR_PARSER_GRDDL
check-local: check-grddl
else
check-local:
endif

if MAINTAINER_MODE
check_grddl_deps = $(TEST_FILES)
endif

check-grddl: build-rapper $(check_grddl_deps)
	@result=0; \
	$(RECHO) "Testing GRDDL"; \
	printf 'IF(RAPTOR_PARSER_GRDDL)\n\n' >>CMakeTests.txt; \
	for test in $(TEST_FILES); do \
	  name=`basename $$test .html` ; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -f noNet -q -i grddl -o ntriples $(srcdir)/$$test > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status != 0 -a $$status != 2 ; then \
	    $(RECHO) FAILED returned status $$status; result=1; \
	  elif cmp $(srcdir)/$$name.out $$name.res >/dev/null 2>&1; then \
	    if test $$status = 2 ; then \
	      $(RECHO) "ok with warnings"; grep Warning $$name.err; \
	    else \
	      $(RECHO) "ok"; \
	    fi; \
	  else \
	    $(RECHO) "FAILED"; \
	    diff $(srcdir)/$$name.out $$name.res; result=1; \
	  fi; \
	  if test $$status != 1 ; then \
	    rm -f $$name.res $$name.err; \
	  fi; \
	  printf '\tRAPPER_TEST(%s\n\t\t"%s"\n\t\t%s\n\t\t%s\n\t)\n\n' \
		grddl.$$name \
		"\$${RAPPER} -f noNet -q -i grddl -o ntriples \$${CMAKE_CURRENT_SOURCE_DIR}/$$test" \
		$$name.res \
		"\$${CMAKE_CURRENT_SOURCE_DIR}/$$name.out" >>CMakeTests.txt; \
	done; \
	printf 'ENDIF(RAPTOR_PARSER_GRDDL)\n\n' >>CMakeTests.txt; \
	exit $$result

# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor TRiG tests
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

TEST_FILES=\
example1.trig example2.trig example3.trig bug370.trig bug451.trig \
bug-584-dos.trig bug-584-unix.trig issue-37.trig

TEST_OUT_FILES=\
example1.out example2.out example3.out bug370.out bug451.out \
bug-584-dos.out bug-584-unix.out issue-37.out

# Used to make N-triples output consistent
BASE_URI=http://example.librdf.org/

EXTRA_DIST = \
	CMakeLists.txt \
	$(TEST_FILES) \
	$(TEST_OUT_FILES)

RAPPER = $(top_builddir)/utils/rapper

CLEANFILES = CMakeTests.txt

build-rapper:
	@(cd $(top_builddir)/utils ; $(MAKE) rapper$(EXEEXT))

check-local: check-trig

if MAINTAINER_MODE
check_trig_deps = $(TEST_FILES)
endif

check-trig: build-rapper $(check_trig_deps)
	@set +e; result=0; \
	$(RECHO) "Testing legal TRiG"; \
	base="$@" ; \
	for test in $(TEST_FILES); do \
	  name=`basename $$test .trig` ; \
	  baseuri=$(BASE_URI)$$test; \
	  expected="$(srcdir)/$$name.out"; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i trig -o nquads $(srcdir)/$$test $$baseuri > $$base.res 2> $$base.err; \
	  status=$$?; \
	  if test $$status != 0 -a $$status != 2 ; then \
	    $(RECHO) FAILED returned status $$status; result=1; \
	    $(RECHO) $(RAPPER) -q -i trig -o nquads $(srcdir)/$$test $$baseuri '>' $$base.res; \
	    cat $$base.err $$base.res; \
	  elif cmp $$expected $$base.res >/dev/null 2>&1; then \
	    if test $$status = 2 ; then \
	      $(RECHO) "ok with warnings"; grep Warning $$base.err; \
	    else \
	      $(RECHO) "ok"; \
	    fi; \
	  else \
	    $(RECHO) "FAILED"; \
	    diff $$expected $$base.res; result=1; \
	  fi; \
	  printf 'RAPPER_TEST(%s\n\t"%s"\n\t%s\n\t%s\n)\n\n' \
		trig.$$name \
		"\$${RAPPER} -q -i trig -o nquads \$${CMAKE_CURRENT_SOURCE_DIR}/$$test $$baseuri" \
		$$base.res \
		"\$${CMAKE_CURRENT_SOURCE_DIR}/$$base.out" >>CMakeTests.txt; \
	done; \
	rm -f $$base.res $$base.err; \
	set -e; exit $$result

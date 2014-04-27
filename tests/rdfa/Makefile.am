# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor RDFa 1.0 tests
#
# Copyright (C) 2008-2012, David Beckett http://purl.org/net/dajobe/
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

BASE_URI=http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/
#
# Manifest:
# https://raw.github.com/rdfa/rdfa-website/master/manifest.ttl
# defines whether sparql ASK should return true or false for success
#
# Query to return all RDFA 1.0 test URIs:
#
# roqet -r csv -D https://raw.github.com/rdfa/rdfa-website/master/manifest.ttl -e 'select ?t where {?t <http://rdfa.info/vocabs/rdfa-test#rdfaVersion> "rdfa1.0"}'
#
# Query to return RDFa 1.0 tests that must return False:
# roqet -r csv -D https://raw.github.com/rdfa/rdfa-website/master/manifest.ttl -e 'select * where {?s <http://www.w3.org/2006/03/test-description#expectedResults> false; <http://rdfa.info/vocabs/rdfa-test#rdfaVersion> "rdfa1.0"}'
#
# http://rdfa.info/test-suite/test-cases/0042
# http://rdfa.info/test-suite/test-cases/0107
# http://rdfa.info/test-suite/test-cases/0122
# http://rdfa.info/test-suite/test-cases/0140
# http://rdfa.info/test-suite/test-cases/0209
# http://rdfa.info/test-suite/test-cases/0211
# http://rdfa.info/test-suite/test-cases/0258
# http://rdfa.info/test-suite/test-cases/0294
#
# These all expect 0 ntriples - failure is >0 triples or ERROR

CLEANFILES= \
CMakeTests.txt \
*-res.nt

TEST_FILES= \
0001.xml 0006.xml 0007.xml 0008.xml 0009.xml 0010.xml 0012.xml \
0013.xml 0014.xml 0015.xml 0017.xml 0018.xml 0019.xml 0020.xml \
0021.xml 0023.xml 0025.xml 0026.xml 0027.xml 0029.xml 0030.xml \
0031.xml 0032.xml 0033.xml 0034.xml 0035.xml 0036.xml 0037.xml \
0038.xml 0039.xml 0040.xml 0041.xml 0042.xml 0046.xml 0047.xml \
0048.xml 0049.xml 0050.xml 0051.xml 0052.xml 0053.xml 0054.xml \
0055.xml 0056.xml 0057.xml 0058.xml 0059.xml 0060.xml 0061.xml \
0062.xml 0063.xml 0064.xml 0065.xml 0066.xml 0067.xml 0068.xml \
0069.xml 0070.xml 0071.xml 0072.xml 0073.xml 0074.xml 0075.xml \
0076.xml 0077.xml 0078.xml 0079.xml 0080.xml 0081.xml 0082.xml \
0083.xml 0084.xml 0085.xml 0087.xml 0088.xml 0089.xml 0090.xml \
0091.xml 0093.xml 0099.xml 0104.xml 0106.xml 0107.xml 0108.xml \
0109.xml 0110.xml 0111.xml 0112.xml 0113.xml 0114.xml 0115.xml \
0117.xml 0118.xml 0119.xml 0120.xml 0121.xml 0122.xml 0126.xml \
0131.xml 0134.xml 0140.xml 0147.xml 0172.xml 0173.xml 0174.xml \
0181.xml 0197.xml 0201.xml 0202.xml 0203.xml 0207.xml 0209.xml \
0210.xml 0211.xml 0212.xml 0215.xml 0258.xml 0262.xml 0291.xml \
0294.xml 0304.xml

TEST_OUT_FILES = \
0001.out 0006.out 0007.out 0008.out 0009.out 0010.out 0012.out \
0013.out 0014.out 0015.out 0017.out 0018.out 0019.out 0020.out \
0021.out 0023.out 0025.out 0026.out 0027.out 0029.out 0030.out \
0031.out 0032.out 0033.out 0034.out 0035.out 0036.out 0037.out \
0038.out 0039.out 0040.out 0041.out 0042.out 0046.out 0047.out \
0048.out 0049.out 0050.out 0051.out 0052.out 0053.out 0054.out \
0055.out 0056.out 0057.out 0058.out 0059.out 0060.out 0061.out \
0062.out 0063.out 0064.out 0065.out 0066.out 0067.out 0068.out \
0069.out 0070.out 0071.out 0072.out 0073.out 0074.out 0075.out \
0076.out 0077.out 0078.out 0079.out 0080.out 0081.out 0082.out \
0083.out 0084.out 0085.out 0087.out 0088.out 0089.out 0090.out \
0091.out 0093.out 0099.out 0104.out 0106.out 0107.out 0108.out \
0109.out 0110.out 0111.out 0112.out 0113.out 0114.out 0115.out \
0117.out 0118.out 0119.out 0120.out 0121.out 0122.out 0126.out \
0131.out 0134.out 0140.out 0147.out 0172.out 0173.out 0174.out \
0181.out 0197.out 0201.out 0202.out 0203.out 0207.out 0209.out \
0210.out 0211.out 0212.out 0215.out 0258.out 0262.out 0291.out \
0294.out 0304.out

ALL_TEST_FILES= \
	$(TEST_FILES) \
	$(TEST_OUT_FILES)

# Expected test failures:
#  0202.xml - @xml:base support in SVG
#  0203.xml - @xml:base in non-root in SVG
#  0294.xml - property="xml:" generates a triple but shouldn't
#  0304.xml - embedded rdf/xml in SVG
# (Add a space to the start of this string)
EXPECTED_FAILURES=" 0202.xml 0203.xml 0294.xml 0304.xml"

EXTRA_DIST = CMakeLists.txt $(ALL_TEST_FILES)

RAPPER  = $(top_builddir)/utils/rapper
RDFDIFF = $(top_builddir)/utils/rdfdiff

build-rapper:
	@(cd $(top_builddir)/utils ; $(MAKE) rapper$(EXEEXT))

build-rdfdiff:
	@(cd $(top_builddir)/utils ; $(MAKE) rdfdiff$(EXEEXT))

if RAPTOR_PARSER_RDFA
check-local: check-rdfa
else
check-local:
endif

if MAINTAINER_MODE
check_rdfa_deps = $(TEST_FILES)
endif

check-rdfa: build-rdfdiff build-rapper $(check_rdfa_deps)
	@set +e; result=0; failures=; failure_count=0; \
	$(RECHO) "Testing RDFa 1.0"; \
	printf 'IF(RAPTOR_PARSER_RDFA)\n\n' >>CMakeTests.txt; \
	for test in $(TEST_FILES); do \
	  name=`basename $$test .xml` ; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  baseuri=$(BASE_URI)$$test; \
	  $(RAPPER) -f noNet -q -i rdfa10 -I $$baseuri -o ntriples $(srcdir)/$$test > $$name-res.nt 2> $$name.err; \
	  status1=$$?; \
	  if test $$status1 = 2; then \
	    status1=0; \
	  fi; \
	  $(RDFDIFF) -f ntriples -u $$baseuri -t ntriples $(srcdir)/$$name.out $$name-res.nt > $$name.res 2> $$name.err; \
	  status2=$$?; \
	  if test $$status1 = 0 -a $$status2 = 0; then \
	    $(RECHO) "ok"; \
	  else \
	    $(RECHO) "FAILED"; \
	    $(RECHO) "Triple counts: expected: `wc -l < $(srcdir)/$$name.out` got: `wc -l < $$name-res.nt` "; \
	    $(RECHO) $(RAPPER) -f noNet -q -i rdfa10 -I $$baseuri -o ntriples $(srcdir)/$$test '>' $$name-res.nt; \
	    $(RECHO) $(RDFDIFF) -f ntriples -u $$baseuri -t ntriples $(srcdir)/$$name.out $$name-res.nt '>' $$name.res; \
	    cat $$name.err $$name.res; \
	    failures="$$failures $$test"; \
	    failure_count=`expr $$failure_count + 1`; \
	  fi; \
	  rm -f $$name-res.nt $$name.res $$name.err; \
	  printf '\tRAPPER_RDFDIFF_TEST(%s\n\t\t"%s"\n\t\t%s\n\t\t"%s"\n\t)\n\n' \
		rdfa.$$name \
		"\$${RAPPER} -f noNet -q -i rdfa10 -I $$baseuri -o ntriples \$${CMAKE_CURRENT_SOURCE_DIR}/$$test" \
		$$name-res.nt \
		"\$${RDFDIFF} -f ntriples -u $$baseuri -t ntriples \$${CMAKE_CURRENT_SOURCE_DIR}/$$name.out $$name-res.nt" \
		>>CMakeTests.txt; \
	done; \
	printf 'ENDIF(RAPTOR_PARSER_RDFA)\n\n' >>CMakeTests.txt; \
	if test "X$$failures" != X; then \
	  if test "$$failures" = $(EXPECTED_FAILURES); then \
	    echo "Ignoring expected failures"; \
	    result=0; \
	  else \
	    echo "FAILED: Got $$failures but expected $(EXPECTED_FAILURES)"; \
	    result=1; \
	  fi; \
	fi; \
	set -e; exit $$result

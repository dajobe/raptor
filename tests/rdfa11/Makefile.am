# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor RDFa 1.1 tests
#
# Copyright (C) 2012, David Beckett http://purl.org/net/dajobe/
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
# Query to return all RDFA 1.1 test URIs:
#
# roqet -r csv -D https://raw.github.com/rdfa/rdfa-website/master/manifest.ttl -e 'select ?t where {?t <http://rdfa.info/vocabs/rdfa-test#rdfaVersion> "rdfa1.1"}'
#
# Query to return RDFa 1.1 tests that must return False:
# roqet -r csv -D https://raw.github.com/rdfa/rdfa-website/master/manifest.ttl -e 'select * where {?s <http://www.w3.org/2006/03/test-description#expectedResults> false; <http://rdfa.info/vocabs/rdfa-test#rdfaVersion> "rdfa1.1"}'
#
# http://rdfa.info/test-suite/test-cases/0107
# http://rdfa.info/test-suite/test-cases/0122
# http://rdfa.info/test-suite/test-cases/0140
# http://rdfa.info/test-suite/test-cases/0235
# http://rdfa.info/test-suite/test-cases/0258
#
# http://rdfa.info/test-suite/test-cases/0180 MUST not return given triple; expected to return 1 triple
#
# These all expect 0 ntriples - failure is >0 triples or ERROR

CLEANFILES= \
CMakeTests.txt \
*-res.nt

TEST_FILES= \
0001.xml 0006.xml 0007.xml 0008.xml 0009.xml 0010.xml 0012.xml	\
0013.xml 0014.xml 0015.xml 0017.xml 0018.xml 0019.xml 0020.xml	\
0021.xml 0023.xml 0025.xml 0026.xml 0027.xml 0029.xml 0030.xml	\
0031.xml 0032.xml 0033.xml 0034.xml 0035.xml 0036.xml 0037.xml	\
0038.xml 0039.xml 0041.xml 0048.xml 0049.xml 0050.xml 0051.xml	\
0052.xml 0053.xml 0054.xml 0055.xml 0056.xml 0057.xml 0059.xml	\
0060.xml 0061.xml 0062.xml 0063.xml 0064.xml 0065.xml 0066.xml	\
0067.xml 0068.xml 0069.xml 0070.xml 0071.xml 0072.xml 0073.xml	\
0074.xml 0075.xml 0076.xml 0077.xml 0079.xml 0080.xml 0083.xml	\
0084.xml 0085.xml 0087.xml 0088.xml 0089.xml 0091.xml 0093.xml	\
0099.xml 0104.xml 0106.xml 0107.xml 0108.xml 0109.xml 0110.xml	\
0111.xml 0112.xml 0113.xml 0114.xml 0115.xml 0117.xml 0118.xml	\
0119.xml 0120.xml 0121.xml 0122.xml 0126.xml 0131.xml 0134.xml	\
0140.xml 0147.xml 0172.xml 0173.xml 0174.xml 0175.xml 0176.xml	\
0177.xml 0178.xml 0179.xml 0180.xml 0181.xml 0182.xml 0183.xml	\
0186.xml 0187.xml 0188.xml 0189.xml 0190.xml 0196.xml 0197.xml	\
0198.xml 0201.xml 0202.xml 0203.xml 0206.xml 0207.xml 0213.xml	\
0214.xml 0216.xml 0217.xml 0218.xml 0219.xml 0220.xml 0221.xml	\
0222.xml 0223.xml 0224.xml 0225.xml 0226.xml 0227.xml 0228.xml	\
0229.xml 0230.xml 0231.xml 0232.xml 0233.xml 0234.xml 0235.xml	\
0236.xml 0237.xml 0238.xml 0239.xml 0246.xml 0247.xml 0248.xml	\
0249.xml 0250.xml 0251.xml 0252.xml 0253.xml 0254.xml 0255.xml	\
0256.xml 0257.xml 0258.xml 0259.xml 0260.xml 0261.xml 0262.xml	\
0263.xml 0264.xml 0265.xml 0266.xml 0267.xml 0268.xml 0269.xml	\
0269.xml 0271.xml 0272.xml 0273.xml 0274.xml 0275.xml 0276.xml	\
0277.xml 0278.xml 0279.xml 0280.xml 0281.xml 0282.xml 0283.xml	\
0284.xml 0285.xml 0286.xml 0287.xml 0289.xml 0290.xml 0291.xml	\
0292.xml 0293.xml 0295.xml 0296.xml 0297.xml 0298.xml 0299.xml	\
0300.xml 0301.xml 0302.xml 0303.xml 0304.xml

TEST_OUT_FILES = \
0001.out 0006.out 0007.out 0008.out 0009.out 0010.out 0012.out	\
0013.out 0014.out 0015.out 0017.out 0018.out 0019.out 0020.out	\
0021.out 0023.out 0025.out 0026.out 0027.out 0029.out 0030.out	\
0031.out 0032.out 0033.out 0034.out 0035.out 0036.out 0037.out	\
0038.out 0039.out 0041.out 0048.out 0049.out 0050.out 0051.out	\
0052.out 0053.out 0054.out 0055.out 0056.out 0057.out 0059.out	\
0060.out 0061.out 0062.out 0063.out 0064.out 0065.out 0066.out	\
0067.out 0068.out 0069.out 0070.out 0071.out 0072.out 0073.out	\
0074.out 0075.out 0076.out 0077.out 0079.out 0080.out 0083.out	\
0084.out 0085.out 0087.out 0088.out 0089.out 0091.out 0093.out	\
0099.out 0104.out 0106.out 0107.out 0108.out 0109.out 0110.out	\
0111.out 0112.out 0113.out 0114.out 0115.out 0117.out 0118.out	\
0119.out 0120.out 0121.out 0122.out 0126.out 0131.out 0134.out	\
0140.out 0147.out 0172.out 0173.out 0174.out 0175.out 0176.out	\
0177.out 0178.out 0179.out 0180.out 0181.out 0182.out 0183.out	\
0186.out 0187.out 0188.out 0189.out 0190.out 0196.out 0197.out	\
0198.out 0201.out 0202.out 0203.out 0206.out 0207.out 0213.out	\
0214.out 0216.out 0217.out 0218.out 0219.out 0220.out 0221.out	\
0222.out 0223.out 0224.out 0225.out 0226.out 0227.out 0228.out	\
0229.out 0230.out 0231.out 0232.out 0233.out 0234.out 0235.out	\
0236.out 0237.out 0238.out 0239.out 0246.out 0247.out 0248.out	\
0249.out 0250.out 0251.out 0252.out 0253.out 0254.out 0255.out	\
0256.out 0257.out 0258.out 0259.out 0260.out 0261.out 0262.out	\
0263.out 0264.out 0265.out 0266.out 0267.out 0268.out 0269.out	\
0269.out 0271.out 0272.out 0273.out 0274.out 0275.out 0276.out	\
0277.out 0278.out 0279.out 0280.out 0281.out 0282.out 0283.out	\
0284.out 0285.out 0286.out 0287.out 0289.out 0290.out 0291.out	\
0292.out 0293.out 0295.out 0296.out 0297.out 0298.out 0299.out	\
0300.out 0301.out 0302.out 0303.out 0304.out

ALL_TEST_FILES= \
	$(TEST_FILES) \
	$(TEST_OUT_FILES)

# Expected test failures (24):
# Test  Problem    Description
# ----------------------------
# 0190  librdfa    fails term case insensitivity 
# 0198  raptor     XML (HTML) markup differences but same meaning
# 0202  librdfa    ensure @xml:base in SVG and XHTML5
# 0203  librdfa    ensure @, XML+RDFa in non-root elements in SVG and XHTML5
# 0236  librdfa    should return triple(?s a <http://www.w3.org/ns/rdfa#Error>)
# 0238  librdfa    should return triple(?s a <http://www.w3.org/ns/rdfa#UnresolvedTerm>)
# 0237 0239
#       librdfa    should return triple(?s a <http://www.w3.org/ns/rdfa#UnresolvedCURIE>)
# 0256  librdfa    wrong language code "hu" instead of "fr"
# 0272 0273 0274 0275 0276 0277 0278 0279 0280 0281 0282
#       librdfa    datatype support
# 0285  librdfa    time element with @datetime an invalid datatype ...
# 0286  librdfa    fails to let @value override @content in data
# 0287  librdfa    datatype (@datetime etc.) attribute value
# 0304  ???        requires running RDF/XML parse for SVG metadata
#
# NOTE: Some tests such as 0295 blow up rdfdiff with stack overflow
# which is why the 'cmp' is done first.
#
# (Add a space to the start of this string)
EXPECTED_FAILURES=" 0190.xml 0198.xml 0202.xml 0203.xml 0236.xml 0237.xml 0238.xml 0239.xml 0256.xml 0272.xml 0273.xml 0274.xml 0275.xml 0276.xml 0277.xml 0278.xml 0279.xml 0280.xml 0281.xml 0282.xml 0285.xml 0286.xml 0287.xml 0304.xml"

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
	$(RECHO) "Testing RDFa 1.1"; \
	printf 'IF(RAPTOR_PARSER_RDFA)\n\n' >>CMakeTests.txt; \
	for test in $(TEST_FILES); do \
	  name=`basename $$test .xml` ; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  baseuri=$(BASE_URI)$$test; \
	  $(RAPPER) -f noNet -q -i rdfa11 -I $$baseuri -o ntriples $(srcdir)/$$test > $$name-res.nt 2> $$name.err; \
	  status1=$$?; \
	  if test $$status1 = 2; then \
	    status1=0; \
	  fi; \
	  if cmp $(srcdir)/$$name.out $$name-res.nt >/dev/null 2>&1; then \
	    status2=0; \
	  else \
	    $(RDFDIFF) -f ntriples -u $$baseuri -t ntriples $(srcdir)/$$name.out $$name-res.nt > $$name.res 2> $$name.err; \
	    status2=$$?; \
	  fi; \
	  if test $$status1 = 0 -a $$status2 = 0; then \
	    $(RECHO) "ok"; \
	  else \
	    $(RECHO) "FAILED"; \
	    $(RECHO) "Triple counts: expected: `wc -l < $(srcdir)/$$name.out` got: `wc -l < $$name-res.nt` "; \
	    $(RECHO) $(RAPPER) -f noNet -q -i rdfa11 -I $$baseuri -o ntriples $(srcdir)/$$test '>' $$name-res.nt; \
	    $(RECHO) $(RDFDIFF) -f ntriples -u $$baseuri -t ntriples $(srcdir)/$$name.out $$name-res.nt '>' $$name.res; \
	    cat $$name.err $$name.res; \
	    failures="$$failures $$test"; \
	    failure_count=`expr $$failure_count + 1`; \
	  fi; \
	  rm -f $$name-res.nt $$name.res $$name.err; \
	  printf '\tRAPPER_RDFDIFF_TEST(%s\n\t\t"%s"\n\t\t%s\n\t\t"%s"\n\t)\n\n' \
		rdfa11.$$name \
		"\$${RAPPER} -f noNet -q -i rdfa11 -I $$baseuri -o ntriples \$${CMAKE_CURRENT_SOURCE_DIR}/$$test" \
		$$name-res.nt \
		"\$${RDFDIFF} -f ntriples -u $$baseuri -t ntriples \$${CMAKE_CURRENT_SOURCE_DIR}/$$name.out $$name-res.nt" \
		>>CMakeTests.txt; \
	done; \
	printf 'ENDIF(RAPTOR_PARSER_RDFA)\n\n' >>CMakeTests.txt; \
	if test "X$$failures" != X; then \
	  echo "$$failure_count tests FAILED: $$failures"; \
	  if test "$$failures" = $(EXPECTED_FAILURES); then \
	    echo "Ignoring expected failures"; \
	    result=0; \
	  else \
	    echo "FAILED: Got:"; echo "  $$failures"; echo "but expected"; echo "  $(EXPECTED_FAILURES)"; \
	    result=1; \
	  fi; \
	fi; \
	set -e; exit $$result

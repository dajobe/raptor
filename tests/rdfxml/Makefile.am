# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor RDF/XML tests
#
# Copyright (C) 2000-2008, David Beckett http://purl.org/net/dajobe/
# Copyright (C) 2000-2004, University of Bristol, UK http://www.bristol.ac.uk/
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

# These are errors in strict mode, warnings in lax - for now
RDF_BAGID_TEST_FILES=ex-03.rdf ex-42.rdf ex-43.rdf ex-44.rdf ex-45.rdf

RDF_TEST_FILES=ex-00.rdf ex-01.rdf ex-02.rdf ex-04.rdf ex-05.rdf \
ex-06.rdf ex-07.rdf ex-08.rdf ex-09.rdf ex-10.rdf ex-11.rdf ex-12.rdf \
ex-13.rdf ex-14.rdf ex-15.rdf ex-16.rdf ex-17.rdf ex-18.rdf \
ex-20.rdf ex-21.rdf ex-22.rdf ex-23.rdf ex-24.rdf ex-25.rdf ex-26.rdf \
ex-27.rdf ex-28.rdf ex-29.rdf ex-30.rdf ex-31.rdf ex-32.rdf ex-33.rdf \
ex-34.rdf ex-35.rdf ex-36.rdf ex-37.rdf ex-38.rdf ex-39.rdf ex-40.rdf \
ex-41.rdf ex-46.rdf ex-47.rdf ex-48.rdf ex-49.rdf ex-51.rdf \
ex-53.rdf ex-54.rdf ex-56.rdf ex-57.rdf ex-58.rdf ex-61.rdf \
ex-62.rdf ex-63.rdf \
22-rdf-syntax-ns.rdf rdfs-namespace.rdf rdf-schema.rdf \
owl-schema.rdf daml-oil.rdf wine.rdf

# ex-55 fails with libxml2
#   libxml2 bug report: http://bugs.gnome.org/show_bug.cgi?id=159219
RDF_MAYFAIL_XML_TEST_FILES=ex-55.rdf

RDF_BAD_TEST_FILES=bad-00.rdf bad-01.rdf bad-02.rdf bad-03.rdf \
bad-04.rdf bad-05.rdf bad-06.rdf bad-07.rdf bad-08.rdf bad-09.rdf \
bad-10.rdf bad-11.rdf bad-12.rdf bad-13.rdf bad-14.rdf bad-18.rdf \
bad-19.rdf bad-20.rdf bad-21.rdf bad-22.rdf bad-23.rdf

RDF_BAD_NFC_TEST_FILES=bad-15.rdf bad-16.rdf bad-17.rdf

RDF_WARN_TEST_FILES=warn-00.rdf warn-02.rdf warn-04.rdf \
warn-05.rdf warn-06.rdf warn-07.rdf \
$(RDF_BAGID_TEST_FILES)

RDF_ASSUME_TEST_FILES=ex-19.rdf

RDF_SCAN_TEST_FILES=ex-52.svg

RDF_OUT_FILES=ex-00.out ex-01.out ex-02.out ex-03.out ex-04.out ex-05.out \
ex-06.out ex-07.out ex-08.out ex-09.out ex-10.out ex-11.out ex-12.out \
ex-13.out ex-14.out ex-15.out ex-16.out ex-17.out ex-18.out \
ex-20.out ex-21.out ex-22.out ex-23.out ex-24.out ex-25.out ex-26.out \
ex-27.out ex-28.out ex-29.out ex-30.out ex-31.out ex-32.out ex-33.out \
ex-34.out ex-35.out ex-36.out ex-37.out ex-38.out ex-39.out ex-40.out \
ex-41.out ex-42.out ex-43.out ex-44.out ex-45.out ex-46.out ex-47.out \
ex-48.out ex-49.out ex-51.out ex-53.out ex-54.out ex-55.out ex-56.out \
ex-57.out ex-58.out ex-61.out ex-61.out ex-62.out ex-63.out \
22-rdf-syntax-ns.out rdfs-namespace.out rdf-schema.out \
owl-schema.out daml-oil.out wine.out

RDF_HACK_OUT_FILES=ex-38-rdfxmla.out

RDF_MAYFAIL_XML_OUT_FILES=ex-55.rdf

RDF_WARN_OUT_FILES=warn-00.out warn-02.out warn-04.out \
warn-05.out warn-06.out warn-07.out

RDF_ASSUME_OUT_FILES=ex-19.out

RDF_SCAN_OUT_FILES=ex-52.out

RDF_SERIALIZE_TEST_FILES=ex-59.nt ex-60.nt
RDF_SERIALIZE_OUT_FILES=ex-59.rdf ex-60.rdf


# Used to make N-triples output consistent
BASE_URI=http://librdf.org/raptor/tests/
# for 22-rdf-syntax-ns.rdf
RDF_NS_URI=http://www.w3.org/1999/02/22-rdf-syntax-ns
# for rdfs-namespace.rdf (2002-04-30) and rdf-schema.rdf (2000-03-27)
RDFS_NS_URI=http://www.w3.org/2000/01/rdf-schema

OWL_DOC_URI="http://www.w3.org/2002/07/owl"

DAML_OIL_DOC_URI="http://www.daml.org/2001/03/daml+oil"

OWL_WINE_URI="http://www.w3.org/TR/owl-guide/wine.rdf"

EXTRA_DIST = \
	CMakeLists.txt \
	$(RDF_TEST_FILES) \
	$(RDF_MAYFAIL_XML_TEST_FILES) \
	$(RDF_ASSUME_TEST_FILES) \
	$(RDF_SCAN_TEST_FILES) \
	$(RDF_BAD_TEST_FILES) \
	$(RDF_BAD_NFC_TEST_FILES) \
	$(RDF_WARN_TEST_FILES) \
	$(RDF_OUT_FILES) \
	$(RDF_MAYFAIL_XML_OUT_FILES) \
	$(RDF_ASSUME_OUT_FILES) \
	$(RDF_SCAN_OUT_FILES) \
	$(RDF_WARN_OUT_FILES) \
	$(RDF_SERIALIZE_TEST_FILES) \
	$(RDF_SERIALIZE_OUT_FILES) \
	$(RDF_HACK_OUT_FILES)

RAPPER  = $(top_builddir)/utils/rapper
RDFDIFF = $(top_builddir)/utils/rdfdiff

CLEANFILES = CMakeTests.txt CMakeTmp.txt

build-rapper:
	@(cd $(top_builddir)/utils ; $(MAKE) rapper$(EXEEXT))

build-rdfdiff:
	@(cd $(top_builddir)/utils ; $(MAKE) rdfdiff$(EXEEXT))

check-local: build-rapper \
check-rdf check-mayfail-xml-rdf check-assume-rdf check-scan-rdf \
check-bad-rdf check-bad-nfc-rdf check-warn-rdf \
check-rdfdiff check-rdfxml check-rdfxmla

## Some non-GNU Make programs modify variables that appear in a target's
## dependencies by prepending VPATH to filenames. We don't want this, so
## we only specify the dependencies in maintainer mode, when GNU Make is
## likely being used.
##
if MAINTAINER_MODE
check_rdf_deps = $(RDF_TEST_FILES)
endif

check-rdf: build-rapper $(check_rdf_deps)
	@set +e; result=0; \
	$(RECHO) "Testing legal rdf/xml"; \
	for test in $(RDF_TEST_FILES); do \
	  name=`basename $$test .rdf` ; \
	  if test $$name = 22-rdf-syntax-ns; then \
	    baseuri=$(RDF_NS_URI); \
	  elif test $$name = rdfs-namespace -o $$name = rdf-schema; then \
	    baseuri=$(RDFS_NS_URI); \
	  elif test $$name = owl-schema; then \
	    baseuri=$(OWL_DOC_URI); \
	  elif test $$name = daml-oil; then \
	    baseuri=$(DAML_OIL_DOC_URI); \
	  elif test $$name = wine; then \
	    baseuri=$(OWL_WINE_URI); \
	  else \
	    baseuri=$(BASE_URI)$$name.rdf; \
	  fi; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -o ntriples $(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
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
	  rm -f $$name.res $$name.err; \
	  printf 'RAPPER_TEST(%s\n\t"%s"\n\t%s\n\t%s\n)\n\n' \
		rdfxml.$$name \
		"\$${RAPPER} -q -o ntriples \$${CMAKE_CURRENT_SOURCE_DIR}/$$test $$baseuri" \
		$$name.res \
		"\$${CMAKE_CURRENT_SOURCE_DIR}/$$name.out" >>CMakeTests.txt; \
	done; \
	set -e; exit $$result

if MAINTAINER_MODE
check_assume_rdf_deps = $(RDF_ASSUME_TEST_FILES)
endif

check-assume-rdf: build-rapper $(check_assume_rdf_deps)
	@set +e; result=0; \
	$(RECHO) "Testing rdf/xml known by context"; \
	for test in $(RDF_ASSUME_TEST_FILES); do \
	  name=`basename $$test .rdf` ; \
	  baseuri=$(BASE_URI)$$name.rdf; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -o ntriples file:$(srcdir)/$$test $$baseuri > $$name.res 2>/dev/null; \
	  if cmp $(srcdir)/$$name.out $$name.res >/dev/null 2>&1; then \
	    $(RECHO) "ok"; \
	  else \
	    $(RECHO) "FAILED"; \
	    diff $(srcdir)/$$name.out $$name.res; result=1; \
	  fi; \
	  rm -f $$name.res ; \
	  printf 'RAPPER_TEST(%s\n\t"%s"\n\t%s\n\t%s\n)\n\n' \
		rdfxml.$$name \
		"\$${RAPPER} -q -o ntriples file:\$${CMAKE_CURRENT_SOURCE_DIR}/$$test $$baseuri" \
		$$name.res \
		"\$${CMAKE_CURRENT_SOURCE_DIR}/$$name.out" >>CMakeTests.txt; \
	done; \
	set -e; exit $$result

if MAINTAINER_MODE
check_scan_rdf_deps = $(RDF_SCAN_TEST_FILES)
endif

check-scan-rdf: build-rapper $(check_scan_rdf_deps)
	@set +e; result=0; \
	$(RECHO) "Testing rdf/xml inside other XML"; \
	for test in $(RDF_SCAN_TEST_FILES); do \
	  name=`echo $$test | sed -e 's/\..*//'` ; \
	  baseuri=$(BASE_URI)$$name.rdf; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -f scanForRDF -o ntriples file:$(srcdir)/$$test $$baseuri > $$name.res 2>/dev/null; \
	  if cmp $(srcdir)/$$name.out $$name.res >/dev/null 2>&1; then \
	    $(RECHO) "ok"; \
	  else \
	    $(RECHO) "FAILED"; \
	    diff $(srcdir)/$$name.out $$name.res; result=1; \
	  fi; \
	  rm -f $$name.res ; \
	  printf 'RAPPER_TEST(%s\n\t"%s"\n\t%s\n\t%s\n)\n\n' \
		rdfxml.$$name \
		"\$${RAPPER} -q -f scanForRDF -o ntriples file:\$${CMAKE_CURRENT_SOURCE_DIR}/$$test $$baseuri" \
		$$name.res \
		"\$${CMAKE_CURRENT_SOURCE_DIR}/$$name.out" >>CMakeTests.txt; \
	done; \
	set -e; exit $$result

if MAINTAINER_MODE
check_mayfail_xml_rdf_deps = $(RDF_MAYFAIL_XML_TEST_FILES)
endif

check-mayfail-xml-rdf: build-rapper $(check_mayfail_xml_rdf_deps)
	@set +e; \
	$(RECHO) "Testing rdf/xml (may fail due to XML parser bugs)"; \
	for test in $(RDF_MAYFAIL_XML_TEST_FILES); do \
	  name=`basename $$test .rdf` ; \
	  baseuri=$(BASE_URI)$$name.rdf; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -o ntriples $(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status != 0 -a $$status != 2 ; then \
	    $(RECHO) FAILED returned status $$status; \
	  elif cmp $(srcdir)/$$name.out $$name.res >/dev/null 2>&1; then \
	    if test $$status = 2 ; then \
	      $(RECHO) "ok with warnings"; grep Warning $$name.err; \
	    else \
	      $(RECHO) "ok"; \
	    fi; \
	  else \
	    $(RECHO) "FAILED"; \
	    diff $(srcdir)/$$name.out $$name.res; \
	  fi; \
	  rm -f $$name.res $$name.err; \
	  printf 'RAPPER_TEST(%s\n\t"%s"\n\t%s\n\t%s\n)\n\n' \
		rdfxml.$$name \
		"\$${RAPPER} -q -o ntriples \$${CMAKE_CURRENT_SOURCE_DIR}/$$test $$baseuri" \
		$$name.res \
		"\$${CMAKE_CURRENT_SOURCE_DIR}/$$name.out" >>CMakeTests.txt; \
	done; \
	set -e; exit 0

if MAINTAINER_MODE
check_bad_rdf_deps = $(RDF_BAD_TEST_FILES)
endif

check-bad-rdf: build-rapper $(check_bad_rdf_deps)
	@set +e; result=0; \
	$(RECHO) "Testing that bad rdf/xml fails"; \
	for test in $(RDF_BAD_TEST_FILES); do \
	  name=`basename $$test .rdf` ; \
	  baseuri=$(BASE_URI)$$name.rdf; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -o ntriples file:$(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status -eq 1 ; then \
	    $(RECHO) "ok"; \
	  elif test $$status -eq 2 ; then \
	    $(RECHO) "FAILED - parsing succeeded with a warning"; \
	    cat $$name.res; grep Warning $$name.err; result=1; \
	  elif test $$status -eq 0 ; then \
	    $(RECHO) "FAILED - parsing succeeded but should have failed"; \
	    cat $$name.res; result=1; \
	  else \
	    $(RECHO) "FAILED - parsing failed with unknown status $$status"; \
	    cat $$name.res; result=1; \
	  fi; \
	  rm -f $$name.res $$name.err ; \
	  printf 'ADD_TEST(%s %s) # WILL_FAIL\n' \
		rdfxml.$$name \
		"\$${RAPPER} -q -o ntriples file:\$${CMAKE_CURRENT_SOURCE_DIR}/$$test $$baseuri" >>CMakeTests.txt; \
	  printf '\t%s\n' rdfxml.$$name >>CMakeTmp.txt; \
	done; \
	(printf '\nSET_TESTS_PROPERTIES(\n'; \
	 cat CMakeTmp.txt; \
	 printf '\tPROPERTIES\n\tWILL_FAIL TRUE\n)\n\n') >>CMakeTests.txt; \
	rm -f CMakeTmp.txt; \
	set -e; exit $$result

if MAINTAINER_MODE
check_bad_nfc_rdf_deps = $(RDF_BAD_NFC_TEST_FILES)
endif

check-bad-nfc-rdf: build-rapper $(check_bad_nfc_rdf_deps)
	@set +e; result=0; \
	$(RECHO) "Testing that rdf/xml with bad Unicode NFC fails"; \
	for test in $(RDF_BAD_NFC_TEST_FILES); do \
	  name=`basename $$test .rdf` ; \
	  baseuri=$(BASE_URI)$$name.rdf; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -m strict -o ntriples file:$(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status -eq 1 ; then \
	    $(RECHO) "ok"; \
	  elif test $$status -eq 2 ; then \
	    $(RECHO) "FAILED - parsing succeeded with a warning"; \
	    cat $$name.res; grep Warning $$name.err; result=1; \
	  elif test $$status -eq 0 ; then \
	    $(RECHO) "FAILED - parsing succeeded but should have failed (NFC test)"; \
	    cat $$name.res; result=0; \
	  else \
	    $(RECHO) "FAILED - parsing failed with unknown status $$status"; \
	    cat $$name.res; result=1; \
	  fi; \
	  rm -f $$name.res $$name.err ; \
	  printf 'ADD_TEST(%s %s) # WILL_FAIL\n' \
		rdfxml.$$name \
		"\$${RAPPER} -q -m strict -o ntriples file:\$${CMAKE_CURRENT_SOURCE_DIR}/$$test $$baseuri" >>CMakeTests.txt; \
	  printf '\t%s\n' rdfxml.$$name >>CMakeTmp.txt; \
	done; \
	(printf '\nSET_TESTS_PROPERTIES(\n'; \
	 cat CMakeTmp.txt; \
	 printf '\tPROPERTIES\n\tWILL_FAIL TRUE\n)\n\n') >>CMakeTests.txt; \
	rm -f CMakeTmp.txt; \
	set -e; exit $$result

if MAINTAINER_MODE
check_warn_rdf_deps = $(RDF_WARN_TEST_FILES)
endif

check-warn-rdf: build-rapper $(check_warn_rdf_deps)
	@set +e; result=0; \
	$(RECHO) "Testing rdf/xml with warnings"; \
	for test in $(RDF_WARN_TEST_FILES); do \
	  name=`basename $$test .rdf` ; \
	  baseuri=$(BASE_URI)$$name.rdf; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -o ntriples file:$(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status -eq 1 ; then \
	    $(RECHO) "FAILED - parsing failed when should have warned"; \
	    cat $$name.res; grep Error $$name.err; result=1; \
	  elif test $$status -eq 2 ; then \
	    if cmp $(srcdir)/$$name.out $$name.res >/dev/null 2>&1; then \
	      $(RECHO) "ok"; \
	    else \
	      $(RECHO) "FAILED"; \
	      diff $(srcdir)/$$name.out $$name.res; result=1; \
	    fi; \
	  else \
	    $(RECHO) "FAILED - parsing failed with unknown status $$status"; \
	    cat $$name.res; result=1; \
	  fi; \
	  rm -f $$name.res $$name.err ; \
	  printf 'RAPPER_TEST(%s\n\t"%s"\n\t%s\n\t%s\n)\n\n' \
		rdfxml.$$name \
		"\$${RAPPER} -q -o ntriples file:\$${CMAKE_CURRENT_SOURCE_DIR}/$$test $$baseuri" \
		$$name.res \
		"\$${CMAKE_CURRENT_SOURCE_DIR}/$$name.out" >>CMakeTests.txt; \
	done; \
	set -e; exit $$result

if MAINTAINER_MODE
check_rdfdiff_deps = $(RDF_TEST_FILES)
endif

check-rdfdiff: build-rdfdiff $(check_rdfdiff_deps)
	@set +e; result=0; \
	$(RECHO) "Testing rdfdiff with legal rdf/xml"; \
	for test in $(RDF_TEST_FILES); do \
	  name=`basename $$test .rdf` ; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RDFDIFF) $(srcdir)/$$test $(srcdir)/$$test > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status = 0; then \
	    $(RECHO) "ok"; \
	  else \
	    $(RECHO) "FAILED"; \
	    cat $$name.err; result=1; \
	  fi; \
	  rm -f $$name.res $$name.err; \
	done; \
	set -e; exit $$result

if MAINTAINER_MODE
check_rdfxml_deps = $(RDF_SERIALIZE_TEST_FILES)
endif

check-rdfxml: build-rapper $(check_rdfxml_deps)
	@set +e; result=0; \
	$(RECHO) "Testing rdfxml serialization with legal rdf/xml"; \
	for test in $(RDF_SERIALIZE_TEST_FILES); do \
	  name=`basename $$test .nt` ; \
	  baseuri=-; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i ntriples -o rdfxml $(srcdir)/$$test $$baseuri > $$name-rdfxml.rdf 2> $$name.err; \
	  if cmp $(srcdir)/$$name.rdf $$name-rdfxml.rdf >/dev/null 2>&1; then \
	    $(RECHO) "ok"; \
	  else \
	    $(RECHO) "FAILED"; \
	    diff $(srcdir)/$$name.rdf $$name-rdfxml.rdf; result=1; \
	  fi; \
	  rm -f $$name-rdfxml.rdf $$name.res $$name.err; \
	  printf 'RAPPER_TEST(%s\n\t"%s"\n\t%s\n\t%s\n)\n\n' \
		rdfxml.$$name \
		"\$${RAPPER} -q -i ntriples -o rdfxml \$${CMAKE_CURRENT_SOURCE_DIR}/$$test $$baseuri" \
		$$name-rdfxml.rdf \
		"\$${CMAKE_CURRENT_SOURCE_DIR}/$$name.rdf" >>CMakeTests.txt; \
	done; \
	set -e; exit $$result

if MAINTAINER_MODE
check_rdfxmla_deps = $(RDF_TEST_FILES)
endif

check-rdfxmla: build-rdfdiff build-rapper $(check_rdfxmla_deps)
	@set +e; result=0; \
	$(RECHO) "Testing rdfxml-abbrev serialization with legal rdf/xml"; \
	for test in $(RDF_TEST_FILES); do \
	  name=`basename $$test .rdf` ; \
	  baseuri=-; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -o rdfxml-abbrev $(srcdir)/$$test $$baseuri > $$name-rdfxmla.rdf 2> $$name.err; \
	  status1=$$?; \
	  if test $$test = ex-38.rdf; then \
	    diff $(srcdir)/ex-38-rdfxmla.out $$name-rdfxmla.rdf > $$name.res 2> $$name.err; \
	    status2=$$?; \
	  else \
	    $(RDFDIFF) $(srcdir)/$$test $$name-rdfxmla.rdf > $$name.res 2> $$name.err; \
	    status2=$$?; \
	  fi; \
	  if test $$status1 = 0 -a $$status2 = 0; then \
	    $(RECHO) "ok"; \
	  else \
	    $(RECHO) "FAILED"; \
	    echo $(RAPPER) -q -o rdfxml-abbrev $(srcdir)/$$test $$baseuri '>' $$name-rdfxmla.rdf; \
	  status1=$$?; \
	    echo $(RDFDIFF) $(srcdir)/$$test $$name-rdfxmla.rdf '>' $$name.res; \
	    echo "rapper + rdfdiff output:"; cat $$name.err; result=1; \
	  fi; \
	  rm -f $$name-rdfxmla.rdf $$name.res $$name.err; \
	done; \
	set -e; exit $$result

print-rdf-test-files:
	@echo $(RDF_TEST_FILES) | tr ' ' '\012'


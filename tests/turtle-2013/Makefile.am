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

RDFCOMPARE=$(srcdir)/../../scripts/rdfcompare

RDFXML_DIR = ../rdfxml
NTRIPLES_DIR = ../ntriples

TEST_GOOD_FILES = \
  turtle-syntax-file-01.ttl \
  turtle-syntax-file-02.ttl \
  turtle-syntax-file-03.ttl \
  turtle-syntax-uri-01.ttl \
  turtle-syntax-uri-02.ttl \
  turtle-syntax-uri-03.ttl \
  turtle-syntax-uri-04.ttl \
  turtle-syntax-base-01.ttl \
  turtle-syntax-base-02.ttl \
  turtle-syntax-base-03.ttl \
  turtle-syntax-base-04.ttl \
  turtle-syntax-prefix-01.ttl \
  turtle-syntax-prefix-02.ttl \
  turtle-syntax-prefix-03.ttl \
  turtle-syntax-prefix-04.ttl \
  turtle-syntax-prefix-05.ttl \
  turtle-syntax-prefix-06.ttl \
  turtle-syntax-prefix-07.ttl \
  turtle-syntax-prefix-08.ttl \
  turtle-syntax-prefix-09.ttl \
  turtle-syntax-string-01.ttl \
  turtle-syntax-string-02.ttl \
  turtle-syntax-string-03.ttl \
  turtle-syntax-string-04.ttl \
  turtle-syntax-string-05.ttl \
  turtle-syntax-string-06.ttl \
  turtle-syntax-string-07.ttl \
  turtle-syntax-string-08.ttl \
  turtle-syntax-string-09.ttl \
  turtle-syntax-string-10.ttl \
  turtle-syntax-string-11.ttl \
  turtle-syntax-str-esc-01.ttl \
  turtle-syntax-str-esc-02.ttl \
  turtle-syntax-str-esc-03.ttl \
  turtle-syntax-pname-esc-01.ttl \
  turtle-syntax-pname-esc-02.ttl \
  turtle-syntax-pname-esc-03.ttl \
  turtle-syntax-bnode-01.ttl \
  turtle-syntax-bnode-02.ttl \
  turtle-syntax-bnode-03.ttl \
  turtle-syntax-bnode-04.ttl \
  turtle-syntax-bnode-05.ttl \
  turtle-syntax-bnode-06.ttl \
  turtle-syntax-bnode-07.ttl \
  turtle-syntax-bnode-08.ttl \
  turtle-syntax-bnode-09.ttl \
  turtle-syntax-bnode-10.ttl \
  turtle-syntax-number-01.ttl \
  turtle-syntax-number-02.ttl \
  turtle-syntax-number-03.ttl \
  turtle-syntax-number-04.ttl \
  turtle-syntax-number-05.ttl \
  turtle-syntax-number-06.ttl \
  turtle-syntax-number-07.ttl \
  turtle-syntax-number-08.ttl \
  turtle-syntax-number-09.ttl \
  turtle-syntax-number-10.ttl \
  turtle-syntax-number-11.ttl \
  turtle-syntax-datatypes-01.ttl \
  turtle-syntax-datatypes-02.ttl \
  turtle-syntax-kw-01.ttl \
  turtle-syntax-kw-02.ttl \
  turtle-syntax-kw-03.ttl \
  turtle-syntax-struct-01.ttl \
  turtle-syntax-struct-02.ttl \
  turtle-syntax-struct-03.ttl \
  turtle-syntax-struct-04.ttl \
  turtle-syntax-struct-05.ttl \
  turtle-syntax-lists-01.ttl \
  turtle-syntax-lists-02.ttl \
  turtle-syntax-lists-03.ttl \
  turtle-syntax-lists-04.ttl \
  turtle-syntax-lists-05.ttl \
  turtle-syntax-ln-dots.ttl \
  turtle-syntax-ln-colons.ttl \
  turtle-syntax-ns-dots.ttl \
  turtle-syntax-blank-label.ttl

TEST_BAD_FILES = \
  turtle-syntax-bad-uri-01.ttl \
  turtle-syntax-bad-uri-02.ttl \
  turtle-syntax-bad-uri-03.ttl \
  turtle-syntax-bad-uri-04.ttl \
  turtle-syntax-bad-uri-05.ttl \
  turtle-syntax-bad-prefix-01.ttl \
  turtle-syntax-bad-prefix-02.ttl \
  turtle-syntax-bad-prefix-03.ttl \
  turtle-syntax-bad-prefix-04.ttl \
  turtle-syntax-bad-prefix-05.ttl \
  turtle-syntax-bad-base-01.ttl \
  turtle-syntax-bad-base-02.ttl \
  turtle-syntax-bad-base-03.ttl \
  turtle-syntax-bad-struct-01.ttl \
  turtle-syntax-bad-struct-02.ttl \
  turtle-syntax-bad-struct-03.ttl \
  turtle-syntax-bad-struct-04.ttl \
  turtle-syntax-bad-struct-05.ttl \
  turtle-syntax-bad-struct-06.ttl \
  turtle-syntax-bad-struct-07.ttl \
  turtle-syntax-bad-kw-01.ttl \
  turtle-syntax-bad-kw-02.ttl \
  turtle-syntax-bad-kw-03.ttl \
  turtle-syntax-bad-kw-04.ttl \
  turtle-syntax-bad-kw-05.ttl \
  turtle-syntax-bad-n3-extras-01.ttl \
  turtle-syntax-bad-n3-extras-02.ttl \
  turtle-syntax-bad-n3-extras-03.ttl \
  turtle-syntax-bad-n3-extras-04.ttl \
  turtle-syntax-bad-n3-extras-05.ttl \
  turtle-syntax-bad-n3-extras-06.ttl \
  turtle-syntax-bad-n3-extras-07.ttl \
  turtle-syntax-bad-n3-extras-08.ttl \
  turtle-syntax-bad-n3-extras-09.ttl \
  turtle-syntax-bad-n3-extras-10.ttl \
  turtle-syntax-bad-n3-extras-11.ttl \
  turtle-syntax-bad-n3-extras-12.ttl \
  turtle-syntax-bad-n3-extras-13.ttl \
  turtle-syntax-bad-struct-08.ttl \
  turtle-syntax-bad-struct-09.ttl \
  turtle-syntax-bad-struct-10.ttl \
  turtle-syntax-bad-struct-11.ttl \
  turtle-syntax-bad-struct-12.ttl \
  turtle-syntax-bad-struct-13.ttl \
  turtle-syntax-bad-struct-14.ttl \
  turtle-syntax-bad-struct-15.ttl \
  turtle-syntax-bad-struct-16.ttl \
  turtle-syntax-bad-struct-17.ttl \
  turtle-syntax-bad-lang-01.ttl \
  turtle-syntax-bad-esc-01.ttl \
  turtle-syntax-bad-esc-02.ttl \
  turtle-syntax-bad-esc-03.ttl \
  turtle-syntax-bad-esc-04.ttl \
  turtle-syntax-bad-pname-01.ttl \
  turtle-syntax-bad-pname-02.ttl \
  turtle-syntax-bad-pname-03.ttl \
  turtle-syntax-bad-string-01.ttl \
  turtle-syntax-bad-string-02.ttl \
  turtle-syntax-bad-string-03.ttl \
  turtle-syntax-bad-string-04.ttl \
  turtle-syntax-bad-string-05.ttl \
  turtle-syntax-bad-string-06.ttl \
  turtle-syntax-bad-string-07.ttl \
  turtle-syntax-bad-num-01.ttl \
  turtle-syntax-bad-num-02.ttl \
  turtle-syntax-bad-num-03.ttl \
  turtle-syntax-bad-num-04.ttl \
  turtle-syntax-bad-num-05.ttl \
  turtle-syntax-bad-LITERAL2_with_langtag_and_datatype.ttl \
  turtle-syntax-bad-blank-label-dot-end.ttl \
  turtle-syntax-bad-number-dot-in-anon.ttl \
  turtle-syntax-bad-ln-dash-start.ttl \
  turtle-syntax-bad-ln-escape.ttl \
  turtle-syntax-bad-ln-escape-start.ttl \
  turtle-syntax-bad-ns-dot-end.ttl \
  turtle-syntax-bad-ns-dot-start.ttl \
  turtle-syntax-bad-missing-ns-dot-end.ttl \
  turtle-syntax-bad-missing-ns-dot-start.ttl

TEST_EVAL_FILES = \
  IRI_subject.ttl \
  IRI_with_four_digit_numeric_escape.ttl \
  IRI_with_eight_digit_numeric_escape.ttl \
  IRI_with_all_punctuation.ttl \
  bareword_a_predicate.ttl \
  old_style_prefix.ttl \
  SPARQL_style_prefix.ttl \
  prefixed_IRI_predicate.ttl \
  prefixed_IRI_object.ttl \
  prefix_only_IRI.ttl \
  prefix_with_PN_CHARS_BASE_character_boundaries.ttl \
  prefix_with_non_leading_extras.ttl \
  localName_with_assigned_nfc_bmp_PN_CHARS_BASE_char_boundaries.ttl \
  localName_with_assigned_nfc_PN_CHARS_BASE_character_boundaries.ttl \
  localName_with_nfc_PN_CHARS_BASE_character_boundaries.ttl \
  default_namespace_IRI.ttl \
  prefix_reassigned_and_used.ttl \
  reserved_escaped_localName.ttl \
  percent_escaped_localName.ttl \
  HYPHEN_MINUS_in_localName.ttl \
  underscore_in_localName.ttl \
  localname_with_COLON.ttl \
  localName_with_leading_underscore.ttl \
  localName_with_leading_digit.ttl \
  localName_with_non_leading_extras.ttl \
  old_style_base.ttl \
  SPARQL_style_base.ttl \
  labeled_blank_node_subject.ttl \
  labeled_blank_node_object.ttl \
  labeled_blank_node_with_PN_CHARS_BASE_character_boundaries.ttl \
  labeled_blank_node_with_leading_underscore.ttl \
  labeled_blank_node_with_leading_digit.ttl \
  labeled_blank_node_with_non_leading_extras.ttl \
  anonymous_blank_node_subject.ttl \
  anonymous_blank_node_object.ttl \
  sole_blankNodePropertyList.ttl \
  blankNodePropertyList_as_subject.ttl \
  blankNodePropertyList_as_object.ttl \
  blankNodePropertyList_with_multiple_triples.ttl \
  nested_blankNodePropertyLists.ttl \
  blankNodePropertyList_containing_collection.ttl \
  collection_subject.ttl \
  collection_object.ttl \
  empty_collection.ttl \
  nested_collection.ttl \
  first.ttl \
  last.ttl \
  LITERAL1.ttl \
  LITERAL1_ascii_boundaries.ttl \
  LITERAL1_with_UTF8_boundaries.ttl \
  LITERAL1_all_controls.ttl \
  LITERAL1_all_punctuation.ttl \
  LITERAL_LONG1.ttl \
  LITERAL_LONG1_ascii_boundaries.ttl \
  LITERAL_LONG1_with_UTF8_boundaries.ttl \
  LITERAL_LONG1_with_1_squote.ttl \
  LITERAL_LONG1_with_2_squotes.ttl \
  LITERAL2.ttl \
  LITERAL2_ascii_boundaries.ttl \
  LITERAL2_with_UTF8_boundaries.ttl \
  LITERAL_LONG2.ttl \
  LITERAL_LONG2_ascii_boundaries.ttl \
  LITERAL_LONG2_with_UTF8_boundaries.ttl \
  LITERAL_LONG2_with_1_squote.ttl \
  LITERAL_LONG2_with_2_squotes.ttl \
  literal_with_CHARACTER_TABULATION.ttl \
  literal_with_BACKSPACE.ttl \
  literal_with_LINE_FEED.ttl \
  literal_with_CARRIAGE_RETURN.ttl \
  literal_with_FORM_FEED.ttl \
  literal_with_REVERSE_SOLIDUS.ttl \
  literal_with_escaped_CHARACTER_TABULATION.ttl \
  literal_with_escaped_BACKSPACE.ttl \
  literal_with_escaped_LINE_FEED.ttl \
  literal_with_escaped_CARRIAGE_RETURN.ttl \
  literal_with_escaped_FORM_FEED.ttl \
  literal_with_numeric_escape4.ttl \
  literal_with_numeric_escape8.ttl \
  IRIREF_datatype.ttl \
  prefixed_name_datatype.ttl \
  bareword_integer.ttl \
  bareword_decimal.ttl \
  bareword_double.ttl \
  double_lower_case_e.ttl \
  negative_numeric.ttl \
  positive_numeric.ttl \
  numeric_with_leading_0.ttl \
  literal_true.ttl \
  literal_false.ttl \
  langtagged_non_LONG.ttl \
  langtagged_LONG.ttl \
  lantag_with_subtag.ttl \
  objectList_with_two_objects.ttl \
  predicateObjectList_with_two_objectLists.ttl \
  repeated_semis_at_end.ttl \
  repeated_semis_not_at_end.ttl \
  turtle-eval-struct-01.ttl \
  turtle-eval-struct-02.ttl \
  turtle-subm-01.ttl \
  turtle-subm-02.ttl \
  turtle-subm-03.ttl \
  turtle-subm-04.ttl \
  turtle-subm-05.ttl \
  turtle-subm-06.ttl \
  turtle-subm-07.ttl \
  turtle-subm-08.ttl \
  turtle-subm-09.ttl \
  turtle-subm-10.ttl \
  turtle-subm-11.ttl \
  turtle-subm-12.ttl \
  turtle-subm-13.ttl \
  turtle-subm-14.ttl \
  turtle-subm-15.ttl \
  turtle-subm-16.ttl \
  turtle-subm-17.ttl \
  turtle-subm-18.ttl \
  turtle-subm-19.ttl \
  turtle-subm-20.ttl \
  turtle-subm-21.ttl \
  turtle-subm-22.ttl \
  turtle-subm-23.ttl \
  turtle-subm-24.ttl \
  turtle-subm-25.ttl \
  turtle-subm-26.ttl \
  turtle-subm-27.ttl \
  comment_following_localName.ttl \
  number_sign_following_localName.ttl \
  comment_following_PNAME_NS.ttl \
  number_sign_following_PNAME_NS.ttl \
  LITERAL_LONG2_with_REVERSE_SOLIDUS.ttl \
  two_LITERAL_LONG2s.ttl \
  langtagged_LONG_with_subtag.ttl \
  oops.ttl

TEST_BAD_EVAL_FILES = \
  turtle-eval-bad-01.ttl \
  turtle-eval-bad-02.ttl \
  turtle-eval-bad-03.ttl \
  turtle-eval-bad-04.ttl

TEST_OUT_FILES = \
  HYPHEN_MINUS_in_localName.nt \
  IRIREF_datatype.nt \
  IRI_spo.nt \
  IRI_subject.nt \
  IRI_with_all_punctuation.nt \
  IRI_with_eight_digit_numeric_escape.nt \
  IRI_with_four_digit_numeric_escape.nt \
  LITERAL1.nt \
  LITERAL1_all_controls.nt \
  LITERAL1_all_punctuation.nt \
  LITERAL1_ascii_boundaries.nt \
  LITERAL1_with_UTF8_boundaries.nt \
  LITERAL2.nt \
  LITERAL2_ascii_boundaries.nt \
  LITERAL2_with_UTF8_boundaries.nt \
  LITERAL_LONG1.nt \
  LITERAL_LONG1_ascii_boundaries.nt \
  LITERAL_LONG1_with_1_squote.nt \
  LITERAL_LONG1_with_2_squotes.nt \
  LITERAL_LONG1_with_UTF8_boundaries.nt \
  LITERAL_LONG2.nt \
  LITERAL_LONG2_ascii_boundaries.nt \
  LITERAL_LONG2_with_1_squote.nt \
  LITERAL_LONG2_with_2_squotes.nt \
  LITERAL_LONG2_with_REVERSE_SOLIDUS.nt \
  LITERAL_LONG2_with_UTF8_boundaries.nt \
  LITERAL_with_UTF8_boundaries.nt \
  SPARQL_style_base.nt \
  SPARQL_style_prefix.nt \
  anonymous_blank_node_object.nt \
  anonymous_blank_node_subject.nt \
  bareword_a_predicate.nt \
  bareword_decimal.nt \
  bareword_double.nt \
  bareword_integer.nt \
  blankNodePropertyList_as_object.nt \
  blankNodePropertyList_as_subject.nt \
  blankNodePropertyList_containing_collection.nt \
  blankNodePropertyList_with_multiple_triples.nt \
  collection_object.nt \
  collection_subject.nt \
  comment_following_PNAME_NS.nt \
  comment_following_localName.nt \
  default_namespace_IRI.nt \
  double_lower_case_e.nt \
  empty_collection.nt \
  first.nt \
  labeled_blank_node_object.nt \
  labeled_blank_node_subject.nt \
  labeled_blank_node_with_PN_CHARS_BASE_character_boundaries.nt \
  labeled_blank_node_with_leading_digit.nt \
  labeled_blank_node_with_leading_underscore.nt \
  labeled_blank_node_with_non_leading_extras.nt \
  langtagged_LONG.nt \
  langtagged_LONG_with_subtag.nt \
  langtagged_non_LONG.nt \
  lantag_with_subtag.nt \
  last.nt \
  literal_false.nt \
  literal_true.nt \
  literal_with_BACKSPACE.nt \
  literal_with_CARRIAGE_RETURN.nt \
  literal_with_CHARACTER_TABULATION.nt \
  literal_with_FORM_FEED.nt \
  literal_with_LINE_FEED.nt \
  literal_with_REVERSE_SOLIDUS.nt \
  literal_with_escaped_BACKSPACE.nt \
  literal_with_escaped_CARRIAGE_RETURN.nt \
  literal_with_escaped_CHARACTER_TABULATION.nt \
  literal_with_escaped_FORM_FEED.nt \
  literal_with_escaped_LINE_FEED.nt \
  literal_with_numeric_escape4.nt \
  literal_with_numeric_escape8.nt \
  localName_with_PN_CHARS_BASE_character_boundaries.nt \
  localName_with_assigned_nfc_PN_CHARS_BASE_character_boundaries.nt \
  localName_with_assigned_nfc_bmp_PN_CHARS_BASE_char_boundaries.nt \
  localName_with_leading_digit.nt \
  localName_with_leading_underscore.nt \
  localName_with_nfc_PN_CHARS_BASE_character_boundaries.nt \
  localName_with_non_leading_extras.nt \
  localname_with_COLON.nt \
  negative_numeric.nt \
  nested_blankNodePropertyLists.nt \
  nested_collection.nt \
  number_sign_following_PNAME_NS.nt \
  number_sign_following_localName.nt \
  numeric_with_leading_0.nt \
  objectList_with_two_objects.nt \
  old_style_base.nt \
  old_style_prefix.nt \
  percent_escaped_localName.nt \
  positive_numeric.nt \
  predicateObjectList_with_two_objectLists.nt \
  prefix_only_IRI.nt \
  prefix_reassigned_and_used.nt \
  prefix_with_PN_CHARS_BASE_character_boundaries.nt \
  prefix_with_non_leading_extras.nt \
  prefixed_IRI_object.nt \
  prefixed_IRI_predicate.nt \
  prefixed_name_datatype.nt \
  repeated_semis_at_end.nt \
  repeated_semis_not_at_end.nt \
  reserved_escaped_localName.nt \
  sole_blankNodePropertyList.nt \
  turtle-eval-struct-01.nt \
  turtle-eval-struct-02.nt \
  turtle-subm-01.nt \
  turtle-subm-02.nt \
  turtle-subm-03.nt \
  turtle-subm-04.nt \
  turtle-subm-05.nt \
  turtle-subm-06.nt \
  turtle-subm-07.nt \
  turtle-subm-08.nt \
  turtle-subm-09.nt \
  turtle-subm-10.nt \
  turtle-subm-11.nt \
  turtle-subm-12.nt \
  turtle-subm-13.nt \
  turtle-subm-14.nt \
  turtle-subm-15.nt \
  turtle-subm-16.nt \
  turtle-subm-17.nt \
  turtle-subm-18.nt \
  turtle-subm-19.nt \
  turtle-subm-20.nt \
  turtle-subm-21.nt \
  turtle-subm-22.nt \
  turtle-subm-23.nt \
  turtle-subm-24.nt \
  turtle-subm-25.nt \
  turtle-subm-26.nt \
  turtle-subm-27.nt \
  turtle-syntax-blank-label.nt \
  turtle-syntax-ln-colons.nt \
  turtle-syntax-ln-dots.nt \
  two_LITERAL_LONG2s.nt \
  underscore_in_localName.nt \
  oops.nt

# Used to make N-triples output consistent
BASE_URI=http://www.w3.org/2013/TurtleTests/
# for rdf-schema.ttl
RDF_NS_URI=http://www.w3.org/1999/02/22-rdf-syntax-ns
# for rdfs-namespace.ttl
RDFS_NS_URI=http://www.w3.org/2000/01/rdf-schema

ALL_TEST_FILES= README \
	$(TEST_GOOD_FILES) \
	$(TEST_BAD_FILES) \
	$(TEST_EVAL_FILES) \
	$(TEST_BAD_EVAL_FILES) \
	$(TEST_OUT_FILES)

EXTRA_DIST = $(ALL_TEST_FILES)

RAPPER  = $(top_builddir)/utils/rapper
RDFDIFF = $(top_builddir)/utils/rdfdiff

build-rapper:
	@(cd $(top_builddir)/utils ; $(MAKE) rapper$(EXEEXT))

build-rdfdiff:
	@(cd $(top_builddir)/utils ; $(MAKE) rdfdiff$(EXEEXT))

check-local: check-good-turtle check-bad-turtle check-eval-turtle

if MAINTAINER_MODE
check_good_turtle_deps = $(TEST_GOOD_FILES)
endif

check-good-turtle: build-rapper $(check_good_turtle_deps)
	@set +e; result=0; errors=0; failures=''; \
	$(RECHO) "Testing good Turtle syntax passes"; \
	for test in $(TEST_GOOD_FILES); do \
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

if MAINTAINER_MODE
check_bad_turtle_deps = $(TEST_BAD_FILES) $(TEST_BAD_EVAL_FILES)
endif

check-bad-turtle: build-rapper $(check_bad_turtle_deps)
	@set +e; result=0; errors=0; failures=''; \
	$(RECHO) "Testing bad Turtle fails"; \
	for test in $(TEST_BAD_FILES) $(TEST_BAD_EVAL_FILES) ; do \
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
check_eval_turtle_deps = $(TEST_EVAL_FILES)
endif

check-eval-turtle: build-rapper $(check_eval_turtle_deps)
	@set +e; result=0; errors=0; failures=''; \
	$(RECHO) "Testing eval Turtle"; \
	for test in $(TEST_EVAL_FILES); do \
	  name=`basename $$test .ttl` ; \
	  baseuri=$(BASE_URI)$$test; \
	  $(RECHO) $(RECHO_N) "Checking $$test $(RECHO_C)"; \
	  $(RAPPER) -q -i turtle -o ntriples $(srcdir)/$$name.nt > $$name.nnt; \
	  $(RAPPER) -q -i turtle -o ntriples $(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; \
	  status=$$?; \
	  if test $$status != 0 -a $$status != 2 ; then \
	    $(RECHO) "FAILED returned status $$status"; result=1; \
	    $(RECHO) $(RAPPER) -q -i turtle -o ntriples $(srcdir)/$$test $$baseuri '>' $$name.res; \
	    cat $$name.err; \
	    errors=`expr $$errors + 1`; \
	    failures="$$failures $$test"; \
	  elif $(RDFCOMPARE) $$name.nnt $$name.res >/dev/null 2>&1; then \
	    if test $$status = 2 ; then \
	      $(RECHO) "ok with warnings"; grep Warning $$name.err; \
	    else \
	      $(RECHO) "ok"; \
	    fi; \
	  else \
	    $(RECHO) "FAILED"; result=1; \
	    $(RECHO) $(RAPPER) -q -i turtle -o ntriples $(srcdir)/$$test $$baseuri '>' $$name.res; \
	    cat $$name.err; \
	    diff $$name.nnt $$name.res; \
	    errors=`expr $$errors + 1`; \
	    failures="$$failures $$test"; \
	  fi; \
	  rm -f $$name.res $$name.err $$name.nnt; \
	done; \
	$(RECHO) "Result: $$errors errors: $$failures"; \
	set -e; exit $$result

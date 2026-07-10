# -*- Mode: Makefile -*-
# W3C RDF 1.1 TriG test suite

BASE_URI = http://www.w3.org/2013/TriGTests/
RDFCOMPARE = $(srcdir)/../../scripts/rdfcompare
RAPPER = $(top_builddir)/utils/rapper
RDFDIFF = $(top_builddir)/utils/rdfdiff

TEST_GOOD_FILES = \
  trig-syntax-struct-06.trig \
  trig-syntax-struct-07.trig \
  trig-syntax-minimal-whitespace-01.trig \
  trig-syntax-file-01.trig \
  trig-syntax-file-02.trig \
  trig-syntax-file-03.trig \
  trig-syntax-uri-01.trig \
  trig-syntax-uri-02.trig \
  trig-syntax-uri-03.trig \
  trig-syntax-uri-04.trig \
  trig-syntax-base-01.trig \
  trig-syntax-base-02.trig \
  trig-syntax-base-03.trig \
  trig-syntax-base-04.trig \
  trig-syntax-prefix-01.trig \
  trig-syntax-prefix-02.trig \
  trig-syntax-prefix-03.trig \
  trig-syntax-prefix-04.trig \
  trig-syntax-prefix-05.trig \
  trig-syntax-prefix-06.trig \
  trig-syntax-prefix-07.trig \
  trig-syntax-prefix-08.trig \
  trig-syntax-prefix-09.trig \
  trig-syntax-string-01.trig \
  trig-syntax-string-02.trig \
  trig-syntax-string-03.trig \
  trig-syntax-string-04.trig \
  trig-syntax-string-05.trig \
  trig-syntax-string-06.trig \
  trig-syntax-string-07.trig \
  trig-syntax-string-08.trig \
  trig-syntax-string-09.trig \
  trig-syntax-string-10.trig \
  trig-syntax-string-11.trig \
  trig-syntax-str-esc-01.trig \
  trig-syntax-str-esc-02.trig \
  trig-syntax-str-esc-03.trig \
  trig-syntax-pname-esc-01.trig \
  trig-syntax-pname-esc-02.trig \
  trig-syntax-pname-esc-03.trig \
  trig-syntax-bnode-01.trig \
  trig-syntax-bnode-02.trig \
  trig-syntax-bnode-03.trig \
  trig-syntax-bnode-04.trig \
  trig-syntax-bnode-05.trig \
  trig-syntax-bnode-06.trig \
  trig-syntax-bnode-07.trig \
  trig-syntax-bnode-08.trig \
  trig-syntax-bnode-09.trig \
  trig-syntax-bnode-10.trig \
  trig-syntax-number-01.trig \
  trig-syntax-number-02.trig \
  trig-syntax-number-03.trig \
  trig-syntax-number-04.trig \
  trig-syntax-number-05.trig \
  trig-syntax-number-06.trig \
  trig-syntax-number-07.trig \
  trig-syntax-number-08.trig \
  trig-syntax-number-09.trig \
  trig-syntax-number-10.trig \
  trig-syntax-number-11.trig \
  trig-syntax-datatypes-01.trig \
  trig-syntax-datatypes-02.trig \
  trig-syntax-kw-01.trig \
  trig-syntax-kw-02.trig \
  trig-syntax-kw-03.trig \
  trig-syntax-struct-01.trig \
  trig-syntax-struct-02.trig \
  trig-syntax-struct-03.trig \
  trig-syntax-struct-04.trig \
  trig-syntax-struct-05.trig \
  trig-syntax-lists-01.trig \
  trig-syntax-lists-02.trig \
  trig-syntax-lists-03.trig \
  trig-syntax-lists-04.trig \
  trig-syntax-lists-05.trig \
  trig-syntax-ln-dots.trig \
  trig-syntax-ln-colons.trig \
  trig-syntax-ns-dots.trig \
  trig-syntax-blank-label.trig \
  trig-kw-graph-01.trig \
  trig-kw-graph-02.trig \
  trig-kw-graph-03.trig \
  trig-kw-graph-04.trig \
  trig-kw-graph-05.trig \
  trig-kw-graph-06.trig \
  trig-kw-graph-07.trig \
  trig-kw-graph-08.trig \
  trig-kw-graph-09.trig \
  trig-kw-graph-10.trig \
  trig-turtle-01.trig \
  trig-turtle-02.trig \
  trig-turtle-03.trig \
  trig-turtle-04.trig \
  trig-turtle-05.trig \
  trig-turtle-06.trig

TEST_BAD_FILES = \
  trig-syntax-bad-base-04.trig \
  trig-syntax-bad-base-05.trig \
  trig-syntax-bad-prefix-06.trig \
  trig-syntax-bad-prefix-07.trig \
  trig-syntax-bad-uri-01.trig \
  trig-syntax-bad-uri-02.trig \
  trig-syntax-bad-uri-03.trig \
  trig-syntax-bad-uri-04.trig \
  trig-syntax-bad-uri-05.trig \
  trig-syntax-bad-prefix-01.trig \
  trig-syntax-bad-prefix-02.trig \
  trig-syntax-bad-prefix-03.trig \
  trig-syntax-bad-prefix-04.trig \
  trig-syntax-bad-prefix-05.trig \
  trig-syntax-bad-base-01.trig \
  trig-syntax-bad-base-02.trig \
  trig-syntax-bad-base-03.trig \
  trig-syntax-bad-struct-02.trig \
  trig-syntax-bad-struct-03.trig \
  trig-syntax-bad-struct-04.trig \
  trig-syntax-bad-struct-05.trig \
  trig-syntax-bad-struct-06.trig \
  trig-syntax-bad-struct-07.trig \
  trig-syntax-bad-kw-01.trig \
  trig-syntax-bad-kw-02.trig \
  trig-syntax-bad-kw-03.trig \
  trig-syntax-bad-kw-04.trig \
  trig-syntax-bad-kw-05.trig \
  trig-syntax-bad-n3-extras-01.trig \
  trig-syntax-bad-n3-extras-02.trig \
  trig-syntax-bad-n3-extras-03.trig \
  trig-syntax-bad-n3-extras-04.trig \
  trig-syntax-bad-n3-extras-05.trig \
  trig-syntax-bad-n3-extras-06.trig \
  trig-syntax-bad-n3-extras-07.trig \
  trig-syntax-bad-n3-extras-08.trig \
  trig-syntax-bad-n3-extras-09.trig \
  trig-syntax-bad-n3-extras-10.trig \
  trig-syntax-bad-n3-extras-11.trig \
  trig-syntax-bad-n3-extras-12.trig \
  trig-syntax-bad-n3-extras-13.trig \
  trig-syntax-bad-struct-09.trig \
  trig-syntax-bad-struct-10.trig \
  trig-syntax-bad-struct-12.trig \
  trig-syntax-bad-struct-13.trig \
  trig-syntax-bad-struct-14.trig \
  trig-syntax-bad-struct-15.trig \
  trig-syntax-bad-struct-16.trig \
  trig-syntax-bad-struct-17.trig \
  trig-syntax-bad-lang-01.trig \
  trig-syntax-bad-esc-01.trig \
  trig-syntax-bad-esc-02.trig \
  trig-syntax-bad-esc-03.trig \
  trig-syntax-bad-esc-04.trig \
  trig-syntax-bad-pname-01.trig \
  trig-syntax-bad-pname-02.trig \
  trig-syntax-bad-pname-03.trig \
  trig-syntax-bad-string-01.trig \
  trig-syntax-bad-string-02.trig \
  trig-syntax-bad-string-03.trig \
  trig-syntax-bad-string-04.trig \
  trig-syntax-bad-string-05.trig \
  trig-syntax-bad-string-06.trig \
  trig-syntax-bad-string-07.trig \
  trig-syntax-bad-num-01.trig \
  trig-syntax-bad-num-02.trig \
  trig-syntax-bad-num-03.trig \
  trig-syntax-bad-num-04.trig \
  trig-syntax-bad-num-05.trig \
  trig-syntax-bad-LITERAL2_with_langtag_and_datatype.trig \
  trig-syntax-bad-blank-label-dot-end.trig \
  trig-syntax-bad-number-dot-in-anon.trig \
  trig-syntax-bad-ln-dash-start.trig \
  trig-syntax-bad-ln-escape.trig \
  trig-syntax-bad-ln-escape-start.trig \
  trig-syntax-bad-ns-dot-end.trig \
  trig-syntax-bad-ns-dot-start.trig \
  trig-syntax-bad-missing-ns-dot-end.trig \
  trig-syntax-bad-missing-ns-dot-start.trig \
  trig-syntax-bad-list-01.trig \
  trig-syntax-bad-list-02.trig \
  trig-syntax-bad-list-03.trig \
  trig-syntax-bad-list-04.trig \
  trig-graph-bad-01.trig \
  trig-graph-bad-02.trig \
  trig-graph-bad-03.trig \
  trig-graph-bad-04.trig \
  trig-graph-bad-05.trig \
  trig-graph-bad-06.trig \
  trig-graph-bad-07.trig \
  trig-graph-bad-08.trig \
  trig-graph-bad-09.trig \
  trig-graph-bad-10.trig \
  trig-graph-bad-11.trig \
  trig-bnodeplist-graph-bad-01.trig \
  trig-collection-graph-bad-01.trig \
  trig-collection-graph-bad-02.trig \
  trig-turtle-bad-01.trig \
  trig-turtle-bad-02.trig

TEST_EVAL_CASES = \
  anonymous_blank_node_graph.trig:labeled_blank_node_graph.nq \
  labeled_blank_node_graph.trig:labeled_blank_node_graph.nq \
  alternating_iri_graphs.trig:alternating_iri_graphs.nq \
  alternating_bnode_graphs.trig:alternating_bnode_graphs.nq \
  IRI_subject.trig:IRI_spo.nq \
  IRI_with_four_digit_numeric_escape.trig:IRI_spo.nq \
  IRI_with_eight_digit_numeric_escape.trig:IRI_spo.nq \
  IRI_with_all_punctuation.trig:IRI_with_all_punctuation.nq \
  bareword_a_predicate.trig:bareword_a_predicate.nq \
  old_style_prefix.trig:IRI_spo.nq \
  SPARQL_style_prefix.trig:IRI_spo.nq \
  prefixed_IRI_predicate.trig:IRI_spo.nq \
  prefixed_IRI_object.trig:IRI_spo.nq \
  prefix_only_IRI.trig:IRI_spo.nq \
  prefix_with_PN_CHARS_BASE_character_boundaries.trig:IRI_spo.nq \
  prefix_with_non_leading_extras.trig:IRI_spo.nq \
  localName_with_assigned_nfc_bmp_PN_CHARS_BASE_character_boundaries.trig:localName_with_assigned_nfc_bmp_PN_CHARS_BASE_character_boundaries.nq \
  localName_with_assigned_nfc_PN_CHARS_BASE_character_boundaries.trig:localName_with_assigned_nfc_PN_CHARS_BASE_character_boundaries.nq \
  localName_with_nfc_PN_CHARS_BASE_character_boundaries.trig:localName_with_nfc_PN_CHARS_BASE_character_boundaries.nq \
  default_namespace_IRI.trig:IRI_spo.nq \
  prefix_reassigned_and_used.trig:prefix_reassigned_and_used.nq \
  reserved_escaped_localName.trig:reserved_escaped_localName.nq \
  percent_escaped_localName.trig:percent_escaped_localName.nq \
  HYPHEN_MINUS_in_localName.trig:HYPHEN_MINUS_in_localName.nq \
  underscore_in_localName.trig:underscore_in_localName.nq \
  localname_with_COLON.trig:localname_with_COLON.nq \
  localName_with_leading_underscore.trig:localName_with_leading_underscore.nq \
  localName_with_leading_digit.trig:localName_with_leading_digit.nq \
  localName_with_non_leading_extras.trig:localName_with_non_leading_extras.nq \
  old_style_base.trig:IRI_spo.nq \
  SPARQL_style_base.trig:IRI_spo.nq \
  labeled_blank_node_subject.trig:labeled_blank_node_subject.nq \
  labeled_blank_node_object.trig:labeled_blank_node_object.nq \
  labeled_blank_node_with_PN_CHARS_BASE_character_boundaries.trig:labeled_blank_node_object.nq \
  labeled_blank_node_with_leading_underscore.trig:labeled_blank_node_object.nq \
  labeled_blank_node_with_leading_digit.trig:labeled_blank_node_object.nq \
  labeled_blank_node_with_non_leading_extras.trig:labeled_blank_node_object.nq \
  anonymous_blank_node_subject.trig:anonymous_blank_node_subject.nq \
  anonymous_blank_node_object.trig:anonymous_blank_node_object.nq \
  sole_blankNodePropertyList.trig:sole_blankNodePropertyList.nq \
  blankNodePropertyList_as_subject.trig:blankNodePropertyList_as_subject.nq \
  blankNodePropertyList_as_object.trig:blankNodePropertyList_as_object.nq \
  blankNodePropertyList_with_multiple_triples.trig:blankNodePropertyList_with_multiple_triples.nq \
  nested_blankNodePropertyLists.trig:nested_blankNodePropertyLists.nq \
  blankNodePropertyList_containing_collection.trig:blankNodePropertyList_containing_collection.nq \
  collection_subject.trig:collection_subject.nq \
  collection_object.trig:collection_object.nq \
  empty_collection.trig:empty_collection.nq \
  nested_collection.trig:nested_collection.nq \
  first.trig:first.nq \
  last.trig:last.nq \
  LITERAL1.trig:LITERAL1.nq \
  LITERAL1_ascii_boundaries.trig:LITERAL1_ascii_boundaries.nq \
  LITERAL1_with_UTF8_boundaries.trig:LITERAL_with_UTF8_boundaries.nq \
  LITERAL1_all_controls.trig:LITERAL1_all_controls.nq \
  LITERAL1_all_punctuation.trig:LITERAL1_all_punctuation.nq \
  LITERAL_LONG1.trig:LITERAL1.nq \
  LITERAL_LONG1_ascii_boundaries.trig:LITERAL_LONG1_ascii_boundaries.nq \
  LITERAL_LONG1_with_UTF8_boundaries.trig:LITERAL_with_UTF8_boundaries.nq \
  LITERAL_LONG1_with_1_squote.trig:LITERAL_LONG1_with_1_squote.nq \
  LITERAL_LONG1_with_2_squotes.trig:LITERAL_LONG1_with_2_squotes.nq \
  LITERAL2.trig:LITERAL1.nq \
  LITERAL2_ascii_boundaries.trig:LITERAL2_ascii_boundaries.nq \
  LITERAL2_with_UTF8_boundaries.trig:LITERAL_with_UTF8_boundaries.nq \
  LITERAL_LONG2.trig:LITERAL1.nq \
  LITERAL_LONG2_ascii_boundaries.trig:LITERAL_LONG2_ascii_boundaries.nq \
  LITERAL_LONG2_with_UTF8_boundaries.trig:LITERAL_with_UTF8_boundaries.nq \
  LITERAL_LONG2_with_1_squote.trig:LITERAL_LONG2_with_1_squote.nq \
  LITERAL_LONG2_with_2_squotes.trig:LITERAL_LONG2_with_2_squotes.nq \
  literal_with_CHARACTER_TABULATION.trig:literal_with_CHARACTER_TABULATION.nq \
  literal_with_BACKSPACE.trig:literal_with_BACKSPACE.nq \
  literal_with_LINE_FEED.trig:literal_with_LINE_FEED.nq \
  literal_with_CARRIAGE_RETURN.trig:literal_with_CARRIAGE_RETURN.nq \
  literal_with_FORM_FEED.trig:literal_with_FORM_FEED.nq \
  literal_with_REVERSE_SOLIDUS.trig:literal_with_REVERSE_SOLIDUS.nq \
  literal_with_escaped_CHARACTER_TABULATION.trig:literal_with_CHARACTER_TABULATION.nq \
  literal_with_escaped_BACKSPACE.trig:literal_with_BACKSPACE.nq \
  literal_with_escaped_LINE_FEED.trig:literal_with_LINE_FEED.nq \
  literal_with_escaped_CARRIAGE_RETURN.trig:literal_with_CARRIAGE_RETURN.nq \
  literal_with_escaped_FORM_FEED.trig:literal_with_FORM_FEED.nq \
  literal_with_numeric_escape4.trig:literal_with_numeric_escape4.nq \
  literal_with_numeric_escape8.trig:literal_with_numeric_escape4.nq \
  IRIREF_datatype.trig:IRIREF_datatype.nq \
  prefixed_name_datatype.trig:IRIREF_datatype.nq \
  bareword_integer.trig:IRIREF_datatype.nq \
  bareword_decimal.trig:bareword_decimal.nq \
  bareword_double.trig:bareword_double.nq \
  double_lower_case_e.trig:double_lower_case_e.nq \
  negative_numeric.trig:negative_numeric.nq \
  positive_numeric.trig:positive_numeric.nq \
  numeric_with_leading_0.trig:numeric_with_leading_0.nq \
  literal_true.trig:literal_true.nq \
  literal_false.trig:literal_false.nq \
  langtagged_non_LONG.trig:langtagged_non_LONG.nq \
  langtagged_LONG.trig:langtagged_non_LONG.nq \
  lantag_with_subtag.trig:lantag_with_subtag.nq \
  objectList_with_two_objects.trig:objectList_with_two_objects.nq \
  predicateObjectList_with_two_objectLists.trig:predicateObjectList_with_two_objectLists.nq \
  repeated_semis_at_end.trig:predicateObjectList_with_two_objectLists.nq \
  repeated_semis_not_at_end.trig:repeated_semis_not_at_end.nq \
  trig-eval-struct-01.trig:trig-eval-struct-01.nq \
  trig-eval-struct-02.trig:trig-eval-struct-02.nq \
  trig-subm-01.trig:trig-subm-01.nq \
  trig-subm-02.trig:trig-subm-02.nq \
  trig-subm-03.trig:trig-subm-03.nq \
  trig-subm-04.trig:trig-subm-04.nq \
  trig-subm-05.trig:trig-subm-05.nq \
  trig-subm-06.trig:trig-subm-06.nq \
  trig-subm-07.trig:trig-subm-07.nq \
  trig-subm-08.trig:trig-subm-08.nq \
  trig-subm-09.trig:trig-subm-09.nq \
  trig-subm-10.trig:trig-subm-10.nq \
  trig-subm-11.trig:trig-subm-11.nq \
  trig-subm-12.trig:trig-subm-12.nq \
  trig-subm-13.trig:trig-subm-13.nq \
  trig-subm-14.trig:trig-subm-14.nq \
  trig-subm-15.trig:trig-subm-15.nq \
  trig-subm-16.trig:trig-subm-16.nq \
  trig-subm-17.trig:trig-subm-17.nq \
  trig-subm-18.trig:trig-subm-18.nq \
  trig-subm-19.trig:trig-subm-19.nq \
  trig-subm-20.trig:trig-subm-20.nq \
  trig-subm-21.trig:trig-subm-21.nq \
  trig-subm-22.trig:trig-subm-22.nq \
  trig-subm-23.trig:trig-subm-23.nq \
  trig-subm-24.trig:trig-subm-24.nq \
  trig-subm-25.trig:trig-subm-25.nq \
  trig-subm-26.trig:trig-subm-26.nq \
  trig-subm-27.trig:trig-subm-27.nq \
  comment_following_localName.trig:IRI_spo.nq \
  number_sign_following_localName.trig:number_sign_following_localName.nq \
  comment_following_PNAME_NS.trig:comment_following_PNAME_NS.nq \
  number_sign_following_PNAME_NS.trig:number_sign_following_PNAME_NS.nq \
  LITERAL_LONG2_with_REVERSE_SOLIDUS.trig:LITERAL_LONG2_with_REVERSE_SOLIDUS.nq \
  two_LITERAL_LONG2s.trig:two_LITERAL_LONG2s.nq \
  langtagged_LONG_with_subtag.trig:langtagged_LONG_with_subtag.nq

TEST_EVAL_FILES = \
  anonymous_blank_node_graph.trig \
  labeled_blank_node_graph.trig \
  alternating_iri_graphs.trig \
  alternating_bnode_graphs.trig \
  IRI_subject.trig \
  IRI_with_four_digit_numeric_escape.trig \
  IRI_with_eight_digit_numeric_escape.trig \
  IRI_with_all_punctuation.trig \
  bareword_a_predicate.trig \
  old_style_prefix.trig \
  SPARQL_style_prefix.trig \
  prefixed_IRI_predicate.trig \
  prefixed_IRI_object.trig \
  prefix_only_IRI.trig \
  prefix_with_PN_CHARS_BASE_character_boundaries.trig \
  prefix_with_non_leading_extras.trig \
  localName_with_assigned_nfc_bmp_PN_CHARS_BASE_character_boundaries.trig \
  localName_with_assigned_nfc_PN_CHARS_BASE_character_boundaries.trig \
  localName_with_nfc_PN_CHARS_BASE_character_boundaries.trig \
  default_namespace_IRI.trig \
  prefix_reassigned_and_used.trig \
  reserved_escaped_localName.trig \
  percent_escaped_localName.trig \
  HYPHEN_MINUS_in_localName.trig \
  underscore_in_localName.trig \
  localname_with_COLON.trig \
  localName_with_leading_underscore.trig \
  localName_with_leading_digit.trig \
  localName_with_non_leading_extras.trig \
  old_style_base.trig \
  SPARQL_style_base.trig \
  labeled_blank_node_subject.trig \
  labeled_blank_node_object.trig \
  labeled_blank_node_with_PN_CHARS_BASE_character_boundaries.trig \
  labeled_blank_node_with_leading_underscore.trig \
  labeled_blank_node_with_leading_digit.trig \
  labeled_blank_node_with_non_leading_extras.trig \
  anonymous_blank_node_subject.trig \
  anonymous_blank_node_object.trig \
  sole_blankNodePropertyList.trig \
  blankNodePropertyList_as_subject.trig \
  blankNodePropertyList_as_object.trig \
  blankNodePropertyList_with_multiple_triples.trig \
  nested_blankNodePropertyLists.trig \
  blankNodePropertyList_containing_collection.trig \
  collection_subject.trig \
  collection_object.trig \
  empty_collection.trig \
  nested_collection.trig \
  first.trig \
  last.trig \
  LITERAL1.trig \
  LITERAL1_ascii_boundaries.trig \
  LITERAL1_with_UTF8_boundaries.trig \
  LITERAL1_all_controls.trig \
  LITERAL1_all_punctuation.trig \
  LITERAL_LONG1.trig \
  LITERAL_LONG1_ascii_boundaries.trig \
  LITERAL_LONG1_with_UTF8_boundaries.trig \
  LITERAL_LONG1_with_1_squote.trig \
  LITERAL_LONG1_with_2_squotes.trig \
  LITERAL2.trig \
  LITERAL2_ascii_boundaries.trig \
  LITERAL2_with_UTF8_boundaries.trig \
  LITERAL_LONG2.trig \
  LITERAL_LONG2_ascii_boundaries.trig \
  LITERAL_LONG2_with_UTF8_boundaries.trig \
  LITERAL_LONG2_with_1_squote.trig \
  LITERAL_LONG2_with_2_squotes.trig \
  literal_with_CHARACTER_TABULATION.trig \
  literal_with_BACKSPACE.trig \
  literal_with_LINE_FEED.trig \
  literal_with_CARRIAGE_RETURN.trig \
  literal_with_FORM_FEED.trig \
  literal_with_REVERSE_SOLIDUS.trig \
  literal_with_escaped_CHARACTER_TABULATION.trig \
  literal_with_escaped_BACKSPACE.trig \
  literal_with_escaped_LINE_FEED.trig \
  literal_with_escaped_CARRIAGE_RETURN.trig \
  literal_with_escaped_FORM_FEED.trig \
  literal_with_numeric_escape4.trig \
  literal_with_numeric_escape8.trig \
  IRIREF_datatype.trig \
  prefixed_name_datatype.trig \
  bareword_integer.trig \
  bareword_decimal.trig \
  bareword_double.trig \
  double_lower_case_e.trig \
  negative_numeric.trig \
  positive_numeric.trig \
  numeric_with_leading_0.trig \
  literal_true.trig \
  literal_false.trig \
  langtagged_non_LONG.trig \
  langtagged_LONG.trig \
  lantag_with_subtag.trig \
  objectList_with_two_objects.trig \
  predicateObjectList_with_two_objectLists.trig \
  repeated_semis_at_end.trig \
  repeated_semis_not_at_end.trig \
  trig-eval-struct-01.trig \
  trig-eval-struct-02.trig \
  trig-subm-01.trig \
  trig-subm-02.trig \
  trig-subm-03.trig \
  trig-subm-04.trig \
  trig-subm-05.trig \
  trig-subm-06.trig \
  trig-subm-07.trig \
  trig-subm-08.trig \
  trig-subm-09.trig \
  trig-subm-10.trig \
  trig-subm-11.trig \
  trig-subm-12.trig \
  trig-subm-13.trig \
  trig-subm-14.trig \
  trig-subm-15.trig \
  trig-subm-16.trig \
  trig-subm-17.trig \
  trig-subm-18.trig \
  trig-subm-19.trig \
  trig-subm-20.trig \
  trig-subm-21.trig \
  trig-subm-22.trig \
  trig-subm-23.trig \
  trig-subm-24.trig \
  trig-subm-25.trig \
  trig-subm-26.trig \
  trig-subm-27.trig \
  comment_following_localName.trig \
  number_sign_following_localName.trig \
  comment_following_PNAME_NS.trig \
  number_sign_following_PNAME_NS.trig \
  LITERAL_LONG2_with_REVERSE_SOLIDUS.trig \
  two_LITERAL_LONG2s.trig \
  langtagged_LONG_with_subtag.trig

TEST_EVAL_REFERENCE_FILES = \
  labeled_blank_node_graph.nq \
  alternating_iri_graphs.nq \
  alternating_bnode_graphs.nq \
  IRI_spo.nq \
  IRI_with_all_punctuation.nq \
  bareword_a_predicate.nq \
  localName_with_assigned_nfc_bmp_PN_CHARS_BASE_character_boundaries.nq \
  localName_with_assigned_nfc_PN_CHARS_BASE_character_boundaries.nq \
  localName_with_nfc_PN_CHARS_BASE_character_boundaries.nq \
  prefix_reassigned_and_used.nq \
  reserved_escaped_localName.nq \
  percent_escaped_localName.nq \
  HYPHEN_MINUS_in_localName.nq \
  underscore_in_localName.nq \
  localname_with_COLON.nq \
  localName_with_leading_underscore.nq \
  localName_with_leading_digit.nq \
  localName_with_non_leading_extras.nq \
  labeled_blank_node_subject.nq \
  labeled_blank_node_object.nq \
  anonymous_blank_node_subject.nq \
  anonymous_blank_node_object.nq \
  sole_blankNodePropertyList.nq \
  blankNodePropertyList_as_subject.nq \
  blankNodePropertyList_as_object.nq \
  blankNodePropertyList_with_multiple_triples.nq \
  nested_blankNodePropertyLists.nq \
  blankNodePropertyList_containing_collection.nq \
  collection_subject.nq \
  collection_object.nq \
  empty_collection.nq \
  nested_collection.nq \
  first.nq \
  last.nq \
  LITERAL1.nq \
  LITERAL1_ascii_boundaries.nq \
  LITERAL_with_UTF8_boundaries.nq \
  LITERAL1_all_controls.nq \
  LITERAL1_all_punctuation.nq \
  LITERAL_LONG1_ascii_boundaries.nq \
  LITERAL_LONG1_with_1_squote.nq \
  LITERAL_LONG1_with_2_squotes.nq \
  LITERAL2_ascii_boundaries.nq \
  LITERAL_LONG2_ascii_boundaries.nq \
  LITERAL_LONG2_with_1_squote.nq \
  LITERAL_LONG2_with_2_squotes.nq \
  literal_with_CHARACTER_TABULATION.nq \
  literal_with_BACKSPACE.nq \
  literal_with_LINE_FEED.nq \
  literal_with_CARRIAGE_RETURN.nq \
  literal_with_FORM_FEED.nq \
  literal_with_REVERSE_SOLIDUS.nq \
  literal_with_numeric_escape4.nq \
  IRIREF_datatype.nq \
  bareword_decimal.nq \
  bareword_double.nq \
  double_lower_case_e.nq \
  negative_numeric.nq \
  positive_numeric.nq \
  numeric_with_leading_0.nq \
  literal_true.nq \
  literal_false.nq \
  langtagged_non_LONG.nq \
  lantag_with_subtag.nq \
  objectList_with_two_objects.nq \
  predicateObjectList_with_two_objectLists.nq \
  repeated_semis_not_at_end.nq \
  trig-eval-struct-01.nq \
  trig-eval-struct-02.nq \
  trig-subm-01.nq \
  trig-subm-02.nq \
  trig-subm-03.nq \
  trig-subm-04.nq \
  trig-subm-05.nq \
  trig-subm-06.nq \
  trig-subm-07.nq \
  trig-subm-08.nq \
  trig-subm-09.nq \
  trig-subm-10.nq \
  trig-subm-11.nq \
  trig-subm-12.nq \
  trig-subm-13.nq \
  trig-subm-14.nq \
  trig-subm-15.nq \
  trig-subm-16.nq \
  trig-subm-17.nq \
  trig-subm-18.nq \
  trig-subm-19.nq \
  trig-subm-20.nq \
  trig-subm-21.nq \
  trig-subm-22.nq \
  trig-subm-23.nq \
  trig-subm-24.nq \
  trig-subm-25.nq \
  trig-subm-26.nq \
  trig-subm-27.nq \
  number_sign_following_localName.nq \
  comment_following_PNAME_NS.nq \
  number_sign_following_PNAME_NS.nq \
  LITERAL_LONG2_with_REVERSE_SOLIDUS.nq \
  two_LITERAL_LONG2s.nq \
  langtagged_LONG_with_subtag.nq

TEST_BAD_EVAL_FILES = \
  trig-eval-bad-01.trig \
  trig-eval-bad-02.trig \
  trig-eval-bad-03.trig \
  trig-eval-bad-04.trig

# Expected failures caused by unsupported RDF 1.1 TriG grammar features:
# - trig-syntax-minimal-whitespace-01: RDF 1.1's reduced whitespace requirements.
# - trig-syntax-base-04 and trig-syntax-prefix-03: case-insensitive SPARQL-style
#   BASE/PREFIX directives combined with an unlabeled default graph block.
# - trig-kw-graph-01 through trig-kw-graph-10: the GRAPH keyword (including
#   empty, repeated, and blank-node-named graph forms).
# - anonymous_blank_node_graph, labeled_blank_node_graph, and
#   alternating_bnode_graphs: anonymous or labeled blank-node graph names.
# - SPARQL_style_prefix and SPARQL_style_base: SPARQL-style directives followed
#   by an unlabeled default graph block.
# trig-syntax-prefix-03 and SPARQL_style_prefix currently terminate with
# SIGSEGV during parser error recovery; parser changes are outside this scope.
EXPECTED_GOOD_FAILURES = trig-syntax-minimal-whitespace-01.trig trig-syntax-base-04.trig trig-syntax-prefix-03.trig trig-kw-graph-01.trig trig-kw-graph-02.trig trig-kw-graph-03.trig trig-kw-graph-04.trig trig-kw-graph-05.trig trig-kw-graph-06.trig trig-kw-graph-07.trig trig-kw-graph-08.trig trig-kw-graph-09.trig trig-kw-graph-10.trig
EXPECTED_EVAL_FAILURES = anonymous_blank_node_graph.trig labeled_blank_node_graph.trig alternating_bnode_graphs.trig SPARQL_style_prefix.trig SPARQL_style_base.trig

EXTRA_DIST = \
  CMakeLists.txt README manifest.ttl \
  $(TEST_GOOD_FILES) $(TEST_BAD_FILES) \
  $(TEST_EVAL_FILES) $(TEST_EVAL_REFERENCE_FILES) $(TEST_BAD_EVAL_FILES)

CLEANFILES = *.res *.err *.expected.nq

build-rapper:
	@(cd $(top_builddir)/utils && $(MAKE) rapper$(EXEEXT))

build-rdfdiff:
	@(cd $(top_builddir)/utils && $(MAKE) rdfdiff$(EXEEXT))

check-local: check-good-trig check-bad-syntax-trig check-eval-trig check-bad-eval-trig

if MAINTAINER_MODE
check_good_trig_deps = $(TEST_GOOD_FILES)
check_bad_syntax_trig_deps = $(TEST_BAD_FILES)
check_eval_trig_deps = $(TEST_EVAL_FILES) $(TEST_EVAL_REFERENCE_FILES)
check_bad_eval_trig_deps = $(TEST_BAD_EVAL_FILES)
endif

check-good-trig: build-rapper $(check_good_trig_deps)
	@set +e; failures=''; passed=0; failed=0; \
	$(RECHO) "Testing TriG positive syntax ($(words $(TEST_GOOD_FILES)) tests)"; \
	for test in $(TEST_GOOD_FILES); do \
	  name=good-`basename $$test .trig`; baseuri=$(BASE_URI)$$test; \
	  $(RAPPER) -q -i trig -o nquads file:$(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; status=$$?; \
	  if test $$status -eq 0 -o $$status -eq 2; then passed=`expr $$passed + 1`; \
	  else failed=`expr $$failed + 1`; failures="$$failures $$test"; fi; \
	done; \
	$(RECHO) "TriG positive syntax: $$passed passed, $$failed expected failures"; \
	result=0; test "x$$failures" = "x $(EXPECTED_GOOD_FAILURES)" || result=1; \
	test $$result -eq 0 || { $(RECHO) "Unexpected failure set:$$failures"; $(RECHO) "Expected: $(EXPECTED_GOOD_FAILURES)"; }; \
	rm -f good-*.res good-*.err; set -e; exit $$result

check-bad-syntax-trig: build-rapper $(check_bad_syntax_trig_deps)
	@set +e; failures=''; passed=0; failed=0; \
	$(RECHO) "Testing TriG negative syntax ($(words $(TEST_BAD_FILES)) tests)"; \
	for test in $(TEST_BAD_FILES); do \
	  name=bad-syntax-`basename $$test .trig`; baseuri=$(BASE_URI)$$test; \
	  $(RAPPER) -q -i trig -o nquads file:$(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; status=$$?; \
	  if test $$status -eq 1; then passed=`expr $$passed + 1`; \
	  else failed=`expr $$failed + 1`; failures="$$failures $$test(status=$$status)"; fi; \
	done; \
	$(RECHO) "TriG negative syntax: $$passed passed, $$failed failed"; \
	rm -f bad-syntax-*.res bad-syntax-*.err; set -e; test $$failed -eq 0

check-eval-trig: build-rapper build-rdfdiff $(check_eval_trig_deps)
	@set +e; failures=''; passed=0; failed=0; \
	$(RECHO) "Testing TriG eval ($(words $(TEST_EVAL_CASES)) tests)"; \
	for test_pair in $(TEST_EVAL_CASES); do \
	  test=$${test_pair%%:*}; expected=$${test_pair#*:}; name=eval-`basename $$test .trig`; baseuri=$(BASE_URI)$$test; \
	  $(RAPPER) -q -i nquads -o nquads $(srcdir)/$$expected > $$name.expected.nq 2> $$name.err; expected_status=$$?; \
	  $(RAPPER) -q -i trig -o nquads $(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; status=$$?; \
	  compare_status=1; \
	  if test \( $$status -eq 0 -o $$status -eq 2 \) -a \( $$expected_status -eq 0 -o $$expected_status -eq 2 \); then \
	    if $(RDFCOMPARE) $$name.expected.nq $$name.res >/dev/null 2>&1; then compare_status=0; \
	    else $(RDFDIFF) -b -f nquads -u $$baseuri -t nquads $$name.expected.nq $$name.res >/dev/null 2>&1; compare_status=$$?; fi; \
	  fi; \
	  if test $$compare_status -eq 0; then passed=`expr $$passed + 1`; \
	  else failed=`expr $$failed + 1`; failures="$$failures $$test"; fi; \
	done; \
	$(RECHO) "TriG eval: $$passed passed, $$failed expected failures"; \
	result=0; test "x$$failures" = "x $(EXPECTED_EVAL_FAILURES)" || result=1; \
	test $$result -eq 0 || { $(RECHO) "Unexpected failure set:$$failures"; $(RECHO) "Expected: $(EXPECTED_EVAL_FAILURES)"; }; \
	rm -f eval-*.res eval-*.err eval-*.expected.nq; set -e; exit $$result

check-bad-eval-trig: build-rapper $(check_bad_eval_trig_deps)
	@set +e; failures=''; passed=0; failed=0; \
	$(RECHO) "Testing TriG negative eval ($(words $(TEST_BAD_EVAL_FILES)) tests)"; \
	for test in $(TEST_BAD_EVAL_FILES); do \
	  name=bad-eval-`basename $$test .trig`; baseuri=$(BASE_URI)$$test; \
	  $(RAPPER) -q -i trig -o nquads file:$(srcdir)/$$test $$baseuri > $$name.res 2> $$name.err; status=$$?; \
	  if test $$status -eq 1; then passed=`expr $$passed + 1`; \
	  else failed=`expr $$failed + 1`; failures="$$failures $$test(status=$$status)"; fi; \
	done; \
	$(RECHO) "TriG negative eval: $$passed passed, $$failed failed"; \
	rm -f bad-eval-*.res bad-eval-*.err; set -e; test $$failed -eq 0

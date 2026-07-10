# raptor/tests/nquads-2013/CMakeLists.txt
#
# CTest coverage matching tests/nquads-2013/Makefile.am.
#

SET(TEST_GOOD_FILES
	comment_following_triple.nq
	langtagged_string.nq
	lantag_with_subtag.nq
	literal_all_controls.nq
	literal_all_punctuation.nq
	literal_ascii_boundaries.nq
	literal_false.nq
	literal.nq
	literal_true.nq
	literal_with_2_dquotes.nq
	literal_with_2_squotes.nq
	literal_with_BACKSPACE.nq
	literal_with_CARRIAGE_RETURN.nq
	literal_with_CHARACTER_TABULATION.nq
	literal_with_dquote.nq
	literal_with_FORM_FEED.nq
	literal_with_LINE_FEED.nq
	literal_with_numeric_escape4.nq
	literal_with_numeric_escape8.nq
	literal_with_REVERSE_SOLIDUS2.nq
	literal_with_REVERSE_SOLIDUS.nq
	literal_with_squote.nq
	literal_with_UTF8_boundaries.nq
	minimal_whitespace.nq
	nq-syntax-bnode-01.nq
	nq-syntax-bnode-02.nq
	nq-syntax-bnode-03.nq
	nq-syntax-bnode-04.nq
	nq-syntax-bnode-05.nq
	nq-syntax-bnode-06.nq
	nq-syntax-uri-01.nq
	nq-syntax-uri-02.nq
	nq-syntax-uri-03.nq
	nq-syntax-uri-04.nq
	nq-syntax-uri-05.nq
	nq-syntax-uri-06.nq
	nt-syntax-bnode-01.nq
	nt-syntax-bnode-02.nq
	nt-syntax-bnode-03.nq
	nt-syntax-datatypes-01.nq
	nt-syntax-datatypes-02.nq
	nt-syntax-file-01.nq
	nt-syntax-file-02.nq
	nt-syntax-file-03.nq
	nt-syntax-str-esc-01.nq
	nt-syntax-str-esc-02.nq
	nt-syntax-str-esc-03.nq
	nt-syntax-string-01.nq
	nt-syntax-string-02.nq
	nt-syntax-string-03.nq
	nt-syntax-subm-01.nq
	nt-syntax-uri-01.nq
	nt-syntax-uri-02.nq
	nt-syntax-uri-03.nq
	nt-syntax-uri-04.nq
)

SET(TEST_BAD_FILES
	nq-syntax-bad-literal-01.nq
	nq-syntax-bad-literal-02.nq
	nq-syntax-bad-literal-03.nq
	nq-syntax-bad-quint-01.nq
	nq-syntax-bad-uri-01.nq
	nt-syntax-bad-base-01.nq
	nt-syntax-bad-esc-01.nq
	nt-syntax-bad-esc-02.nq
	nt-syntax-bad-esc-03.nq
	nt-syntax-bad-lang-01.nq
	nt-syntax-bad-num-01.nq
	nt-syntax-bad-num-02.nq
	nt-syntax-bad-num-03.nq
	nt-syntax-bad-prefix-01.nq
	nt-syntax-bad-string-01.nq
	nt-syntax-bad-string-02.nq
	nt-syntax-bad-string-03.nq
	nt-syntax-bad-string-04.nq
	nt-syntax-bad-string-05.nq
	nt-syntax-bad-string-06.nq
	nt-syntax-bad-string-07.nq
	nt-syntax-bad-struct-01.nq
	nt-syntax-bad-struct-02.nq
	nt-syntax-bad-uri-01.nq
	nt-syntax-bad-uri-02.nq
	nt-syntax-bad-uri-03.nq
	nt-syntax-bad-uri-04.nq
	nt-syntax-bad-uri-05.nq
	nt-syntax-bad-uri-06.nq
	nt-syntax-bad-uri-07.nq
	nt-syntax-bad-uri-08.nq
	nt-syntax-bad-uri-09.nq
)

SET(BASE_URI http://www.w3.org/2013/N-QuadsTests/)

IF(RAPTOR_PARSER_NQUADS AND RAPTOR_SERIALIZER_NQUADS)
	FOREACH(TEST_FILE ${TEST_GOOD_FILES})
		GET_FILENAME_COMPONENT(TEST_NAME ${TEST_FILE} NAME_WE)
		SET(TEST_PREFIX nquads-2013.good.${TEST_NAME})
		RAPPER_PARSE_TEST(${TEST_PREFIX}
			"${RAPPER} -q -i nquads -o nquads file:${CMAKE_CURRENT_SOURCE_DIR}/${TEST_FILE} ${BASE_URI}${TEST_FILE}"
		)
	ENDFOREACH(TEST_FILE)

	FOREACH(TEST_FILE ${TEST_BAD_FILES})
		GET_FILENAME_COMPONENT(TEST_NAME ${TEST_FILE} NAME_WE)
		SET(TEST_PREFIX nquads-2013.bad.${TEST_NAME})
		RAPPER_EXPECT_FAIL_TEST(${TEST_PREFIX}
			"${RAPPER} -q -i nquads -o nquads file:${CMAKE_CURRENT_SOURCE_DIR}/${TEST_FILE} ${BASE_URI}${TEST_FILE}"
		)
	ENDFOREACH(TEST_FILE)
ENDIF(RAPTOR_PARSER_NQUADS AND RAPTOR_SERIALIZER_NQUADS)

# end raptor/tests/nquads-2013/CMakeLists.txt


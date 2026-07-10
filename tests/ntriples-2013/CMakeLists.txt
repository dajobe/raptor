# raptor/tests/ntriples-2013/CMakeLists.txt
#
# CTest coverage matching tests/ntriples-2013/Makefile.am.
#

SET(TEST_GOOD_FILES
	nt-syntax-file-01.nt
	nt-syntax-file-02.nt
	nt-syntax-file-03.nt
	nt-syntax-uri-01.nt
	nt-syntax-uri-02.nt
	nt-syntax-uri-03.nt
	nt-syntax-uri-04.nt
	nt-syntax-string-01.nt
	nt-syntax-string-02.nt
	nt-syntax-string-03.nt
	nt-syntax-str-esc-01.nt
	nt-syntax-str-esc-02.nt
	nt-syntax-str-esc-03.nt
	nt-syntax-bnode-01.nt
	nt-syntax-bnode-02.nt
	nt-syntax-bnode-03.nt
	nt-syntax-datatypes-01.nt
	nt-syntax-datatypes-02.nt
	nt-syntax-subm-01.nt
	comment_following_triple.nt
	LITERAL.nt
	LITERAL_all_controls.nt
	LITERAL_all_punctuation.nt
	LITERAL_ascii_boundaries.nt
	literal_false.nt
	literal_true.nt
	LITERAL_with_2_dquotes.nt
	LITERAL_with_2_squotes.nt
	literal_with_BACKSPACE.nt
	literal_with_CARRIAGE_RETURN.nt
	literal_with_CHARACTER_TABULATION.nt
	literal_with_FORM_FEED.nt
	literal_with_LINE_FEED.nt
	literal_with_REVERSE_SOLIDUS.nt
	literal_with_REVERSE_SOLIDUS2.nt
	LITERAL_with_UTF8_boundaries.nt
	LITERAL_with_dquote.nt
	literal_with_numeric_escape4.nt
	literal_with_numeric_escape8.nt
	LITERAL_with_squote.nt
	langtagged_string.nt
	lantag_with_subtag.nt
	minimal_whitespace.nt
)

SET(TEST_BAD_FILES
	nt-syntax-bad-uri-01.nt
	nt-syntax-bad-uri-02.nt
	nt-syntax-bad-uri-03.nt
	nt-syntax-bad-uri-04.nt
	nt-syntax-bad-uri-05.nt
	nt-syntax-bad-uri-06.nt
	nt-syntax-bad-uri-07.nt
	nt-syntax-bad-uri-08.nt
	nt-syntax-bad-uri-09.nt
	nt-syntax-bad-prefix-01.nt
	nt-syntax-bad-base-01.nt
	nt-syntax-bad-struct-01.nt
	nt-syntax-bad-struct-02.nt
	nt-syntax-bad-lang-01.nt
	nt-syntax-bad-esc-01.nt
	nt-syntax-bad-esc-02.nt
	nt-syntax-bad-esc-03.nt
	nt-syntax-bad-string-01.nt
	nt-syntax-bad-string-02.nt
	nt-syntax-bad-string-03.nt
	nt-syntax-bad-string-04.nt
	nt-syntax-bad-string-05.nt
	nt-syntax-bad-string-06.nt
	nt-syntax-bad-string-07.nt
	nt-syntax-bad-num-01.nt
	nt-syntax-bad-num-02.nt
	nt-syntax-bad-num-03.nt
)

SET(BASE_URI http://www.w3.org/2013/N-TriplesTests/)

IF(RAPTOR_PARSER_NTRIPLES AND RAPTOR_SERIALIZER_NTRIPLES)
	FOREACH(TEST_FILE ${TEST_GOOD_FILES})
		GET_FILENAME_COMPONENT(TEST_NAME ${TEST_FILE} NAME_WE)
		SET(TEST_PREFIX ntriples-2013.good.${TEST_NAME})
		RAPPER_PARSE_TEST(${TEST_PREFIX}
			"${RAPPER} -q -i ntriples -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/${TEST_FILE} ${BASE_URI}${TEST_FILE}"
		)
	ENDFOREACH(TEST_FILE)

	FOREACH(TEST_FILE ${TEST_BAD_FILES})
		GET_FILENAME_COMPONENT(TEST_NAME ${TEST_FILE} NAME_WE)
		SET(TEST_PREFIX ntriples-2013.bad.${TEST_NAME})
		RAPPER_EXPECT_FAIL_TEST(${TEST_PREFIX}
			"${RAPPER} -q -i ntriples -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/${TEST_FILE} ${BASE_URI}${TEST_FILE}"
		)
	ENDFOREACH(TEST_FILE)
ENDIF(RAPTOR_PARSER_NTRIPLES AND RAPTOR_SERIALIZER_NTRIPLES)

# end raptor/tests/ntriples-2013/CMakeLists.txt


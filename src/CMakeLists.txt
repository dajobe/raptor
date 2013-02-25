# raptor/src/CMakeLists.txt
#
# Original listfile by Daniel Richard G. <skunk@iSKUNK.ORG>
# This file is in the public domain.
#

IF(RAPTOR_PARSER_RDFA)
	INCLUDE_DIRECTORIES(BEFORE ${CMAKE_SOURCE_DIR}/librdfa)
ENDIF(RAPTOR_PARSER_RDFA)

IF(RAPTOR_PARSER_RDFXML)
	SET(raptor_parser_rdfxml_sources raptor_rdfxml.c)
	SET(raptor_libxml_libs ${LIBXML2_LIBRARIES})
ENDIF(RAPTOR_PARSER_RDFXML)
IF(RAPTOR_PARSER_NTRIPLES OR RAPTOR_PARSER_NQUADS)
	SET(raptor_parser_ntriples_nquads_sources ntriples_parse.c)
ENDIF(RAPTOR_PARSER_NTRIPLES OR RAPTOR_PARSER_NQUADS)
IF(RAPTOR_PARSER_TURTLE OR RAPTOR_PARSER_TRIG)
	SET(raptor_parser_turtle_trig_sources
		turtle_common.h
		turtle_lexer.c
		turtle_lexer.h
		turtle_parser.c
		turtle_parser.h
	)
ENDIF(RAPTOR_PARSER_TURTLE OR RAPTOR_PARSER_TRIG)
IF(RAPTOR_PARSER_RSS OR RAPTOR_SERIALIZER_RSS_1_0)
	SET(raptor_rss_common_sources raptor_rss_common.c raptor_rss.h)
ENDIF(RAPTOR_PARSER_RSS OR RAPTOR_SERIALIZER_RSS_1_0)
IF(RAPTOR_PARSER_RSS)
	SET(raptor_parser_rss_sources raptor_rss.c)
	SET(raptor_libxml_libs ${LIBXML2_LIBRARIES})
ENDIF(RAPTOR_PARSER_RSS)
IF(RAPTOR_PARSER_GRDDL)
	SET(raptor_parser_grddl_sources raptor_grddl.c)
	SET(raptor_libxml_libs ${LIBXML2_LIBRARIES})
	SET(raptor_libxslt_libs ${LIBXSLT_LIBRARIES})
ENDIF(RAPTOR_PARSER_GRDDL)
IF(RAPTOR_PARSER_GUESS)
	SET(raptor_parser_guess_sources raptor_guess.c)
ENDIF(RAPTOR_PARSER_GUESS)
IF(RAPTOR_PARSER_RDFA)
	SET(raptor_parser_rdfa_sources raptor_librdfa.c)
ENDIF(RAPTOR_PARSER_RDFA)
IF(RAPTOR_PARSER_JSON)
	SET(raptor_parser_json_sources raptor_json.c)
ENDIF(RAPTOR_PARSER_JSON)

IF(RAPTOR_SERIALIZER_RDFXML)
	SET(raptor_serializer_rdfxml_sources raptor_serialize_rdfxml.c)
ENDIF(RAPTOR_SERIALIZER_RDFXML)
IF(RAPTOR_SERIALIZER_NTRIPLES OR RAPTOR_SERIALIZER_NQUADS)
	SET(raptor_serializer_ntriples_nquads_sources raptor_serialize_ntriples.c)
ENDIF(RAPTOR_SERIALIZER_NTRIPLES OR RAPTOR_SERIALIZER_NQUADS)
IF(RAPTOR_SERIALIZER_RDFXML_ABBREV OR RAPTOR_SERIALIZER_TURTLE)
	SET(raptor_serializer_abbrev_sources raptor_abbrev.c)
ENDIF(RAPTOR_SERIALIZER_RDFXML_ABBREV OR RAPTOR_SERIALIZER_TURTLE)
IF(RAPTOR_SERIALIZER_RDFXML_ABBREV)
	SET(raptor_serializer_rdfxml_abbrev_sources raptor_serialize_rdfxmla.c)
ENDIF(RAPTOR_SERIALIZER_RDFXML_ABBREV)
IF(RAPTOR_SERIALIZER_TURTLE)
	SET(raptor_serializer_turtle_sources raptor_serialize_turtle.c)
ENDIF(RAPTOR_SERIALIZER_TURTLE)
IF(RAPTOR_SERIALIZER_RSS_1_0)
	SET(raptor_serializer_rss_1_0_sources raptor_serialize_rss.c)
ENDIF(RAPTOR_SERIALIZER_RSS_1_0)
IF(RAPTOR_SERIALIZER_DOT)
	SET(raptor_serializer_dot_sources raptor_serialize_dot.c)
ENDIF(RAPTOR_SERIALIZER_DOT)
IF(RAPTOR_SERIALIZER_HTML)
	SET(raptor_serializer_html_sources raptor_serialize_html.c)
ENDIF(RAPTOR_SERIALIZER_HTML)
IF(RAPTOR_SERIALIZER_JSON)
	SET(raptor_serializer_json_sources raptor_serialize_json.c)
	SET(raptor_yajl_libs ${YAJL_LIBRARIES})
ENDIF(RAPTOR_SERIALIZER_JSON)

IF(RAPTOR_WWW STREQUAL "curl")
	SET(raptor_www_sources raptor_www_curl.c)
	SET(raptor_www_libs ${CURL_LIBRARIES})
ELSEIF(RAPTOR_WWW STREQUAL "fetch")
	SET(raptor_www_sources raptor_www_libfetch.c)
	#SET(raptor_www_libs ${FETCH_LIBRARIES})
ELSEIF(RAPTOR_WWW STREQUAL "xml")
	SET(raptor_www_sources raptor_www_libxml.c)
	SET(raptor_www_libs ${LIBXML2_LIBRARIES})
ENDIF(RAPTOR_WWW STREQUAL "curl")

IF(RAPTOR_XML STREQUAL "libxml")
	SET(raptor_libxml_sources raptor_libxml.c)
	SET(raptor_libxml_libs ${LIBXML2_LIBRARIES})
ENDIF(RAPTOR_XML STREQUAL "libxml")
IF(RAPTOR_PARSER_RDFA)
	SET(raptor_librdfa_sources
		${CMAKE_SOURCE_DIR}/librdfa/context.c
		${CMAKE_SOURCE_DIR}/librdfa/curie.c
		${CMAKE_SOURCE_DIR}/librdfa/iri.c
		${CMAKE_SOURCE_DIR}/librdfa/language.c
		${CMAKE_SOURCE_DIR}/librdfa/lists.c
		${CMAKE_SOURCE_DIR}/librdfa/namespace.c
		${CMAKE_SOURCE_DIR}/librdfa/rdfa.c
		${CMAKE_SOURCE_DIR}/librdfa/rdfa_utils.c
		${CMAKE_SOURCE_DIR}/librdfa/strtok_r.c
		${CMAKE_SOURCE_DIR}/librdfa/subject.c
		${CMAKE_SOURCE_DIR}/librdfa/triple.c
		${CMAKE_SOURCE_DIR}/librdfa/rdfa.h
		${CMAKE_SOURCE_DIR}/librdfa/rdfa_utils.h
		${CMAKE_SOURCE_DIR}/librdfa/strtok_r.h
	)
ENDIF(RAPTOR_PARSER_RDFA)
IF(NOT HAVE_STRCASECMP AND NOT HAVE_STRICMP)
	SET(raptor_strcasecmp_sources strcasecmp.c)
ENDIF(NOT HAVE_STRCASECMP AND NOT HAVE_STRICMP)
IF(RAPTOR_PARSEDATE)
	SET(raptor_parsedate_sources parsedate.c)
ENDIF(RAPTOR_PARSEDATE)

ADD_LIBRARY(raptor2
	raptor_avltree.c
	raptor_concepts.c
	raptor_general.c
	raptor_iostream.c
	raptor_json_writer.c
	raptor_locator.c
	raptor_log.c
	raptor_memstr.c
	raptor_namespace.c
	raptor_option.c
	raptor_parse.c
	raptor_qname.c
	raptor_rfc2396.c
	raptor_sax2.c
	raptor_sequence.c
	raptor_serialize.c
	raptor_set.c
	raptor_statement.c
	raptor_stringbuffer.c
	raptor_syntax_description.c
	raptor_term.c
	raptor_turtle_writer.c
	raptor_unicode.c
	raptor_uri.c
	raptor_www.c
	raptor_xml.c
	raptor_xml_writer.c
	snprintf.c
	turtle_common.c
	${raptor_parser_rdfxml_sources}
	${raptor_parser_ntriples_nquads_sources}
	${raptor_parser_turtle_trig_sources}
	${raptor_rss_common_sources}
	${raptor_parser_rss_sources}
	${raptor_parser_grddl_sources}
	${raptor_parser_guess_sources}
	${raptor_parser_rdfa_sources}
	${raptor_parser_json_sources}
	${raptor_serializer_rdfxml_sources}
	${raptor_serializer_ntriples_nquads_sources}
	${raptor_serializer_abbrev_sources}
	${raptor_serializer_rdfxml_abbrev_sources}
	${raptor_serializer_turtle_sources}
	${raptor_serializer_rss_1_0_sources}
	${raptor_serializer_dot_sources}
	${raptor_serializer_html_sources}
	${raptor_serializer_json_sources}
	${raptor_www_sources}
	${raptor_libxml_sources}
	${raptor_librdfa_sources}
	${raptor_strcasecmp_sources}
	${raptor_parsedate_sources}
)

TARGET_LINK_LIBRARIES(raptor2
	${raptor_libxslt_libs}
	${raptor_libxml_libs}
	${raptor_yajl_libs}
	${raptor_www_libs}
)

SET_TARGET_PROPERTIES(
	raptor2
	PROPERTIES
	COMPILE_DEFINITIONS "LIBRDFA_IN_RAPTOR;RAPTOR_INTERNAL;${LIBXML2_DEFINITIONS};${LIBXSLT_DEFINITIONS}"
)

ADD_EXECUTABLE(turtle_lexer_test turtle_lexer.c)
TARGET_LINK_LIBRARIES(turtle_lexer_test raptor2)

IF(NOT WIN32 OR NOT BUILD_SHARED_LIBS)
	# This currently cannot be built on Windows with DLL linkage
	ADD_EXECUTABLE(turtle_parser_test turtle_parser.c)
	TARGET_LINK_LIBRARIES(turtle_parser_test raptor2)
	SET_TARGET_PROPERTIES(
		turtle_parser_test
		PROPERTIES
		COMPILE_DEFINITIONS "RAPTOR_INTERNAL;STANDALONE"
	)
ENDIF(NOT WIN32 OR NOT BUILD_SHARED_LIBS)

ADD_EXECUTABLE(raptor_parse_test raptor_parse.c)
TARGET_LINK_LIBRARIES(raptor_parse_test raptor2)
ADD_TEST(raptor_parse_test raptor_parse_test)

ADD_EXECUTABLE(raptor_rfc2396_test raptor_rfc2396.c)
TARGET_LINK_LIBRARIES(raptor_rfc2396_test raptor2)
ADD_TEST(raptor_rfc2396_test raptor_rfc2396_test)

ADD_EXECUTABLE(raptor_uri_test raptor_uri.c)
TARGET_LINK_LIBRARIES(raptor_uri_test raptor2)
ADD_TEST(raptor_uri_test raptor_uri_test)

ADD_EXECUTABLE(raptor_namespace_test raptor_namespace.c)
TARGET_LINK_LIBRARIES(raptor_namespace_test raptor2)
ADD_TEST(raptor_namespace_test raptor_namespace_test)

ADD_EXECUTABLE(strcasecmp_test strcasecmp.c)
TARGET_LINK_LIBRARIES(strcasecmp_test raptor2)
ADD_TEST(strcasecmp_test strcasecmp_test)

ADD_EXECUTABLE(raptor_www_test raptor_www_test.c)
TARGET_LINK_LIBRARIES(raptor_www_test raptor2)
ADD_TEST(raptor_www_test raptor_www_test)

ADD_EXECUTABLE(raptor_sequence_test raptor_sequence.c)
TARGET_LINK_LIBRARIES(raptor_sequence_test raptor2)
ADD_TEST(raptor_sequence_test raptor_sequence_test)

ADD_EXECUTABLE(raptor_stringbuffer_test raptor_stringbuffer.c)
TARGET_LINK_LIBRARIES(raptor_stringbuffer_test raptor2)
ADD_TEST(raptor_stringbuffer_test raptor_stringbuffer_test)

ADD_EXECUTABLE(raptor_iostream_test raptor_iostream.c)
TARGET_LINK_LIBRARIES(raptor_iostream_test raptor2)
ADD_TEST(raptor_iostream_test raptor_iostream_test)

ADD_EXECUTABLE(raptor_xml_writer_test raptor_xml_writer.c)
TARGET_LINK_LIBRARIES(raptor_xml_writer_test raptor2)
ADD_TEST(raptor_xml_writer_test raptor_xml_writer_test)

ADD_EXECUTABLE(raptor_turtle_writer_test raptor_turtle_writer.c)
TARGET_LINK_LIBRARIES(raptor_turtle_writer_test raptor2)
ADD_TEST(raptor_turtle_writer_test raptor_turtle_writer_test)

ADD_EXECUTABLE(raptor_avltree_test raptor_avltree.c)
TARGET_LINK_LIBRARIES(raptor_avltree_test raptor2)
ADD_TEST(raptor_avltree_test raptor_avltree_test)

ADD_EXECUTABLE(raptor_term_test raptor_term.c)
TARGET_LINK_LIBRARIES(raptor_term_test raptor2)
ADD_TEST(raptor_term_test raptor_term_test)

ADD_EXECUTABLE(raptor_permute_test raptor_permute_test.c)
TARGET_LINK_LIBRARIES(raptor_permute_test raptor2)
ADD_TEST(raptor_permute_test raptor_permute_test)

SET_TARGET_PROPERTIES(
	turtle_lexer_test
	#turtle_parser_test
	raptor_parse_test
	raptor_rfc2396_test
	raptor_uri_test
	raptor_namespace_test
	strcasecmp_test
	raptor_www_test
	raptor_sequence_test
	raptor_stringbuffer_test
	raptor_iostream_test
	raptor_xml_writer_test
	raptor_turtle_writer_test
	raptor_avltree_test
	raptor_term_test
	raptor_permute_test
	PROPERTIES
	COMPILE_DEFINITIONS "RAPTOR_INTERNAL;STANDALONE"
)

IF(RAPTOR_PARSER_RDFXML)
	ADD_EXECUTABLE(raptor_set_test raptor_set.c)
	TARGET_LINK_LIBRARIES(raptor_set_test raptor2)
	ADD_TEST(raptor_set_test raptor_set_test)

	ADD_EXECUTABLE(raptor_xml_test raptor_xml.c)
	TARGET_LINK_LIBRARIES(raptor_xml_test raptor2)
	ADD_TEST(raptor_xml_test raptor_xml_test)

	SET_TARGET_PROPERTIES(
		raptor_set_test
		raptor_xml_test
		PROPERTIES
		COMPILE_DEFINITIONS "RAPTOR_INTERNAL;STANDALONE"
	)
ENDIF(RAPTOR_PARSER_RDFXML)

# Generate pkg-config metadata file
#
FILE(WRITE ${CMAKE_CURRENT_BINARY_DIR}/raptor2.pc
"prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=\${prefix}
libdir=\${exec_prefix}/lib
includedir=\${prefix}/include/raptor2

Name: Raptor RDF Parsing Library
Description: RDF Parser Toolkit Library
Version: ${VERSION}
Libs: -L\${libdir} -lraptor2
Libs.private: ${raptor_libxslt_libs} ${raptor_libxml_libs}
Cflags: -I\${includedir}
")

INSTALL(FILES
	raptor.h
	${CMAKE_CURRENT_BINARY_DIR}/raptor2.h
	DESTINATION include/raptor2
)

INSTALL(FILES
	${CMAKE_CURRENT_BINARY_DIR}/raptor2.pc
	DESTINATION lib/pkgconfig
)

INSTALL(
	TARGETS raptor2
	EXPORT  Raptor2Config
	ARCHIVE DESTINATION lib
	LIBRARY DESTINATION lib
	RUNTIME DESTINATION bin
)

INSTALL(EXPORT Raptor2Config DESTINATION lib/cmake)

# end raptor/src/CMakeLists.txt

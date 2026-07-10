# raptor/tests/mkr/CMakeLists.txt
#
# CTest coverage matching tests/mkr/Makefile.am.
#

SET(TEST_SERIALIZE_FILES
	rdf-schema.ttl
	rdfs-namespace.ttl
	rdfq-results.ttl
	test-07.ttl
	test-08.ttl
	test-16a.ttl
	test-28.ttl
	test-31.ttl
	test-32.ttl
	test-34.ttl
)

SET(BASE_URI http://www.w3.org/2001/sw/DataAccess/df1/tests/)
SET(RDF_NS_URI http://www.w3.org/1999/02/22-rdf-syntax-ns)
SET(RDFS_NS_URI http://www.w3.org/2000/01/rdf-schema)

IF(RAPTOR_PARSER_TURTLE AND RAPTOR_SERIALIZER_MKR)
	FOREACH(TEST_FILE ${TEST_SERIALIZE_FILES})
		GET_FILENAME_COMPONENT(TEST_NAME ${TEST_FILE} NAME_WE)
		SET(TEST_BASE_URI ${BASE_URI}${TEST_FILE})
		IF(TEST_NAME STREQUAL rdf-schema)
			SET(TEST_BASE_URI ${RDF_NS_URI})
		ELSEIF(TEST_NAME STREQUAL rdfs-namespace)
			SET(TEST_BASE_URI ${RDFS_NS_URI})
		ENDIF()
		SET(TEST_PREFIX mkr.serialize.${TEST_NAME})
		RAPPER_TEST(${TEST_PREFIX}
			"${RAPPER} -q -i turtle -o mkr ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_FILE} ${TEST_BASE_URI}"
			${TEST_PREFIX}.mkr
			${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}.mkr
		)
	ENDFOREACH(TEST_FILE)
ENDIF(RAPTOR_PARSER_TURTLE AND RAPTOR_SERIALIZER_MKR)

# end raptor/tests/mkr/CMakeLists.txt


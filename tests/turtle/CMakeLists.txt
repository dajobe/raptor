# raptor/tests/turtle/CMakeLists.txt
#
# CTest coverage matching tests/turtle/Makefile.am.
#

SET(TEST_FILES
	test-00.ttl
	test-01.ttl
	test-02.ttl
	test-03.ttl
	test-04.ttl
	test-05.ttl
	test-06.ttl
	test-07.ttl
	test-08.ttl
	test-09.ttl
	test-10.ttl
	test-11.ttl
	test-12.ttl
	test-13.ttl
	test-14.ttl
	test-15.ttl
	test-16.ttl
	test-17.ttl
	test-18.ttl
	test-19.ttl
	test-20.ttl
	test-21.ttl
	test-22.ttl
	test-23.ttl
	test-24.ttl
	test-25.ttl
	test-26.ttl
	test-27.ttl
	test-29.ttl
	test-30.ttl
	test-33.ttl
	test-36.ttl
	test-37.ttl
	test-38.ttl
	test-40.ttl
	test-41.ttl
	bad-15.ttl
	bad-17.ttl
	bad-18.ttl
	bad-21.ttl
	bad-22.ttl
	rdf-schema.ttl
	rdfs-namespace.ttl
	rdfq-results.ttl
)

SET(TEST_BAD_FILES
	bad-00.ttl
	bad-01.ttl
	bad-02.ttl
	bad-03.ttl
	bad-04.ttl
	bad-05.ttl
	bad-06.ttl
	bad-07.ttl
	bad-08.ttl
	bad-09.ttl
	bad-10.ttl
	bad-11.ttl
	bad-12.ttl
	bad-13.ttl
	bad-14.ttl
	bad-16.ttl
	bad-19.ttl
	bad-20.ttl
	bad-23.ttl
	bad-24.ttl
	bad-25.ttl
	bad-26.ttl
)

SET(TEST_SERIALIZE_FILES
	test-28.ttl
	test-31.ttl
	test-32.ttl
	test-34.ttl
	test-39.ttl
)

SET(NT_TEST_FILES
	test.nt
	nt2013-ex1.nt
	nt2013-ex2.nt
	nt2013-ex3.nt
	nt2013-ex4.nt
	bug-562.nt
	bug-574.nt
	bug-577.nt
	issue77.nt
)

SET(RDF_TEST_FILES
	ex-00.rdf
	ex-01.rdf
	ex-02.rdf
	ex-04.rdf
	ex-05.rdf
	ex-06.rdf
	ex-07.rdf
	ex-08.rdf
	ex-09.rdf
	ex-10.rdf
	ex-11.rdf
	ex-12.rdf
	ex-13.rdf
	ex-14.rdf
	ex-15.rdf
	ex-16.rdf
	ex-17.rdf
	ex-18.rdf
	ex-20.rdf
	ex-21.rdf
	ex-22.rdf
	ex-23.rdf
	ex-24.rdf
	ex-25.rdf
	ex-26.rdf
	ex-27.rdf
	ex-28.rdf
	ex-29.rdf
	ex-30.rdf
	ex-31.rdf
	ex-32.rdf
	ex-33.rdf
	ex-34.rdf
	ex-35.rdf
	ex-36.rdf
	ex-37.rdf
	ex-38.rdf
	ex-39.rdf
	ex-40.rdf
	ex-41.rdf
	ex-46.rdf
	ex-47.rdf
	ex-48.rdf
	ex-49.rdf
	ex-51.rdf
	ex-53.rdf
	ex-54.rdf
	ex-56.rdf
	ex-57.rdf
	ex-58.rdf
	ex-61.rdf
	ex-62.rdf
	ex-63.rdf
	bug-650.rdf
	22-rdf-syntax-ns.rdf
	rdfs-namespace.rdf
	rdf-schema.rdf
	owl-schema.rdf
	daml-oil.rdf
	wine.rdf
)

SET(BASE_URI http://www.w3.org/2001/sw/DataAccess/df1/tests/)
SET(RDF_NS_URI http://www.w3.org/1999/02/22-rdf-syntax-ns)
SET(RDFS_NS_URI http://www.w3.org/2000/01/rdf-schema)

IF(RAPTOR_PARSER_TURTLE AND RAPTOR_SERIALIZER_NTRIPLES)
	FOREACH(TEST_FILE ${TEST_FILES})
		GET_FILENAME_COMPONENT(TEST_NAME ${TEST_FILE} NAME_WE)
		SET(TEST_PREFIX turtle.parse.${TEST_NAME})
		RAPPER_TEST(${TEST_PREFIX}
			"${RAPPER} -q -i turtle -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_FILE} ${BASE_URI}${TEST_FILE}"
			${TEST_PREFIX}.res
			${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}.out
		)
	ENDFOREACH(TEST_FILE)

	FOREACH(TEST_FILE ${TEST_BAD_FILES})
		GET_FILENAME_COMPONENT(TEST_NAME ${TEST_FILE} NAME_WE)
		SET(TEST_PREFIX turtle.bad.${TEST_NAME})
		RAPPER_EXPECT_FAIL_TEST(${TEST_PREFIX}
			"${RAPPER} -q -i turtle -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/${TEST_FILE} ${BASE_URI}${TEST_FILE}"
		)
	ENDFOREACH(TEST_FILE)

	FOREACH(TEST_FILE ${NT_TEST_FILES})
		GET_FILENAME_COMPONENT(TEST_NAME ${TEST_FILE} NAME_WE)
		SET(TEST_PREFIX turtle.ntriples.${TEST_NAME})
		RAPPER_TEST(${TEST_PREFIX}
			"${RAPPER} -q -i turtle -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/../ntriples/${TEST_FILE} -"
			${TEST_PREFIX}.res
			${CMAKE_CURRENT_SOURCE_DIR}/../ntriples/${TEST_NAME}.out
		)
	ENDFOREACH(TEST_FILE)
ENDIF(RAPTOR_PARSER_TURTLE AND RAPTOR_SERIALIZER_NTRIPLES)

IF(RAPTOR_PARSER_TURTLE AND RAPTOR_SERIALIZER_TURTLE)
	FOREACH(TEST_FILE ${TEST_FILES})
		GET_FILENAME_COMPONENT(TEST_NAME ${TEST_FILE} NAME_WE)
		SET(TEST_BASE_URI ${BASE_URI}${TEST_FILE})
		IF(TEST_NAME STREQUAL rdf-schema)
			SET(TEST_BASE_URI ${RDF_NS_URI})
		ELSEIF(TEST_NAME STREQUAL rdfs-namespace)
			SET(TEST_BASE_URI ${RDFS_NS_URI})
		ENDIF()
		SET(TEST_PREFIX turtle.serialize.${TEST_NAME})
		RAPPER_RDFDIFF_TEST(${TEST_PREFIX}
			"${RAPPER} -q -i turtle -o turtle ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_FILE} ${TEST_BASE_URI}"
			${TEST_PREFIX}.ttl
			"${RDFDIFF} -f turtle -u ${TEST_BASE_URI} -t turtle ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_FILE} ${TEST_PREFIX}.ttl"
		)
	ENDFOREACH(TEST_FILE)

	FOREACH(TEST_FILE ${TEST_SERIALIZE_FILES})
		GET_FILENAME_COMPONENT(TEST_NAME ${TEST_FILE} NAME_WE)
		SET(TEST_BASE_URI ${BASE_URI}${TEST_FILE})
		IF(TEST_NAME STREQUAL test-31 OR TEST_NAME STREQUAL test-32 OR TEST_NAME STREQUAL test-39)
			SET(TEST_BASE_URI -)
		ENDIF()
		SET(TEST_PREFIX turtle.syntax.${TEST_NAME})
		RAPPER_TEST(${TEST_PREFIX}
			"${RAPPER} -q -i turtle -o turtle ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_FILE} ${TEST_BASE_URI}"
			${TEST_PREFIX}.ttl
			${CMAKE_CURRENT_SOURCE_DIR}/${TEST_NAME}-out.ttl
		)
	ENDFOREACH(TEST_FILE)
ENDIF(RAPTOR_PARSER_TURTLE AND RAPTOR_SERIALIZER_TURTLE)

IF(RAPTOR_PARSER_RDFXML AND RAPTOR_PARSER_TURTLE AND RAPTOR_SERIALIZER_TURTLE)
	FOREACH(TEST_FILE ${RDF_TEST_FILES})
		GET_FILENAME_COMPONENT(TEST_NAME ${TEST_FILE} NAME_WE)
		SET(TEST_PREFIX turtle.rdfxml.${TEST_NAME})
		IF(TEST_NAME STREQUAL ex-38)
			RAPPER_TEST(${TEST_PREFIX}
				"${RAPPER} -q -o turtle ${CMAKE_CURRENT_SOURCE_DIR}/../rdfxml/${TEST_FILE} -"
				${TEST_PREFIX}.ttl
				${CMAKE_CURRENT_SOURCE_DIR}/ex-38-turtle.out
			)
		ELSE()
			RAPPER_RDFDIFF_TEST(${TEST_PREFIX}
				"${RAPPER} -q -o turtle ${CMAKE_CURRENT_SOURCE_DIR}/../rdfxml/${TEST_FILE} -"
				${TEST_PREFIX}.ttl
				"${RDFDIFF} -t turtle ${CMAKE_CURRENT_SOURCE_DIR}/../rdfxml/${TEST_FILE} ${TEST_PREFIX}.ttl"
			)
		ENDIF()
	ENDFOREACH(TEST_FILE)

	RAPPER_RDFDIFF_TEST(turtle.rdfxml.extra-ex-62
		"${RAPPER} -q -o turtle ${CMAKE_CURRENT_SOURCE_DIR}/ex-62.rdf -"
		turtle.rdfxml.extra-ex-62.ttl
		"${RDFDIFF} -t turtle ${CMAKE_CURRENT_SOURCE_DIR}/ex-62.rdf turtle.rdfxml.extra-ex-62.ttl"
	)
ENDIF(RAPTOR_PARSER_RDFXML AND RAPTOR_PARSER_TURTLE AND RAPTOR_SERIALIZER_TURTLE)

# end raptor/tests/turtle/CMakeLists.txt

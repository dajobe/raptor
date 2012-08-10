# raptor/tests/json/CMakeLists.txt
#
# Original listfile by Daniel Richard G. <skunk@iSKUNK.ORG>
# This file is in the public domain.
#

IF(RAPTOR_PARSER_JSON)

	RAPPER_TEST(json.example1
		"${RAPPER} -q -i json -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/example1.json http://example.librdf.org/example1.json"
		example1.res
		${CMAKE_CURRENT_SOURCE_DIR}/example1.nt
	)

	RAPPER_TEST(json.example2
		"${RAPPER} -q -i json -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/example2.json http://example.librdf.org/example2.json"
		example2.res
		${CMAKE_CURRENT_SOURCE_DIR}/example2.nt
	)

	RAPPER_TEST(json.example3
		"${RAPPER} -q -i json -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/example3.json http://example.librdf.org/example3.json"
		example3.res
		${CMAKE_CURRENT_SOURCE_DIR}/example3.nt
	)

	RAPPER_TEST(json.example4
		"${RAPPER} -q -i json -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/example4.json http://example.librdf.org/example4.json"
		example4.res
		${CMAKE_CURRENT_SOURCE_DIR}/example4.nt
	)

	ADD_TEST(json.bad-00 ${RAPPER} -q -i json -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-00.json http://example.librdf.org/bad-00.json) # WILL_FAIL
	ADD_TEST(json.bad-01 ${RAPPER} -q -i json -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-01.json http://example.librdf.org/bad-01.json) # WILL_FAIL
	ADD_TEST(json.bad-02 ${RAPPER} -q -i json -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-02.json http://example.librdf.org/bad-02.json) # WILL_FAIL
	ADD_TEST(json.bad-03 ${RAPPER} -q -i json -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-03.json http://example.librdf.org/bad-03.json) # WILL_FAIL
	ADD_TEST(json.bad-04 ${RAPPER} -q -i json -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-04.json http://example.librdf.org/bad-04.json) # WILL_FAIL
	ADD_TEST(json.bad-05 ${RAPPER} -q -i json -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-05.json http://example.librdf.org/bad-05.json) # WILL_FAIL
	ADD_TEST(json.bad-06 ${RAPPER} -q -i json -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-06.json http://example.librdf.org/bad-06.json) # WILL_FAIL
	ADD_TEST(json.bad-07 ${RAPPER} -q -i json -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-07.json http://example.librdf.org/bad-07.json) # WILL_FAIL
	ADD_TEST(json.bad-08 ${RAPPER} -q -i json -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-08.json http://example.librdf.org/bad-08.json) # WILL_FAIL
	ADD_TEST(json.bad-09 ${RAPPER} -q -i json -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-09.json http://example.librdf.org/bad-09.json) # WILL_FAIL
	ADD_TEST(json.bad-10 ${RAPPER} -q -i json -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-10.json http://example.librdf.org/bad-10.json) # WILL_FAIL
	ADD_TEST(json.bad-11 ${RAPPER} -q -i json -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-11.json http://example.librdf.org/bad-11.json) # WILL_FAIL
	ADD_TEST(json.bad-12 ${RAPPER} -q -i json -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-12.json http://example.librdf.org/bad-12.json) # WILL_FAIL
	ADD_TEST(json.bad-13 ${RAPPER} -q -i json -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-13.json http://example.librdf.org/bad-13.json) # WILL_FAIL

	SET_TESTS_PROPERTIES(
		json.bad-00
		json.bad-01
		json.bad-02
		json.bad-03
		json.bad-04
		json.bad-05
		json.bad-06
		json.bad-07
		json.bad-08
		json.bad-09
		json.bad-10
		json.bad-11
		json.bad-12
		json.bad-13
		PROPERTIES
		WILL_FAIL TRUE
	)

ENDIF(RAPTOR_PARSER_JSON)

# end raptor/tests/json/CMakeLists.txt

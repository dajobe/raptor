# raptor/tests/bugs/CMakeLists.txt
#
# CTest coverage matching tests/bugs/Makefile.am.
#

ADD_EXECUTABLE(raptor_issue70a_test issue70a.c)
TARGET_INCLUDE_DIRECTORIES(raptor_issue70a_test PRIVATE
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_BINARY_DIR}/src
)
TARGET_COMPILE_DEFINITIONS(raptor_issue70a_test PRIVATE RAPTOR_INTERNAL=1)
TARGET_LINK_LIBRARIES(raptor_issue70a_test raptor2_impl)
ADD_TEST(bugs.issue70a raptor_issue70a_test)

IF(RAPTOR_PARSER_NTRIPLES)
	ADD_EXECUTABLE(raptor_issue70b_test issue70b.c)
	TARGET_INCLUDE_DIRECTORIES(raptor_issue70b_test PRIVATE
		${CMAKE_SOURCE_DIR}/src
		${CMAKE_BINARY_DIR}/src
	)
	TARGET_COMPILE_DEFINITIONS(raptor_issue70b_test PRIVATE RAPTOR_INTERNAL=1)
	TARGET_LINK_LIBRARIES(raptor_issue70b_test raptor2_impl)
	ADD_TEST(bugs.issue70b raptor_issue70b_test)
ENDIF(RAPTOR_PARSER_NTRIPLES)

# end raptor/tests/bugs/CMakeLists.txt

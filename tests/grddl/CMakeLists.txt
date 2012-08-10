# raptor/tests/grddl/CMakeLists.txt
#
# Original listfile by Daniel Richard G. <skunk@iSKUNK.ORG>
# This file is in the public domain.
#

IF(RAPTOR_PARSER_GRDDL)

	RAPPER_TEST(grddl.test-01
		"${RAPPER} -f noNet -q -i grddl -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/test-01.html"
		test-01.res
		${CMAKE_CURRENT_SOURCE_DIR}/test-01.out
	)

ENDIF(RAPTOR_PARSER_GRDDL)

# end raptor/tests/grddl/CMakeLists.txt

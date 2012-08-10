# raptor/tests/trig/CMakeLists.txt
#
# Original listfile by Daniel Richard G. <skunk@iSKUNK.ORG>
# This file is in the public domain.
#

RAPPER_TEST(trig.example1
	"${RAPPER} -q -i trig -o nquads ${CMAKE_CURRENT_SOURCE_DIR}/example1.trig http://example.librdf.org/example1.trig"
	example1.res
	${CMAKE_CURRENT_SOURCE_DIR}/example1.out
)

RAPPER_TEST(trig.example2
	"${RAPPER} -q -i trig -o nquads ${CMAKE_CURRENT_SOURCE_DIR}/example2.trig http://example.librdf.org/example2.trig"
	example2.res
	${CMAKE_CURRENT_SOURCE_DIR}/example2.out
)

RAPPER_TEST(trig.example3
	"${RAPPER} -q -i trig -o nquads ${CMAKE_CURRENT_SOURCE_DIR}/example3.trig http://example.librdf.org/example3.trig"
	example3.res
	${CMAKE_CURRENT_SOURCE_DIR}/example3.out
)

RAPPER_TEST(trig.bug370
	"${RAPPER} -q -i trig -o nquads ${CMAKE_CURRENT_SOURCE_DIR}/bug370.trig http://example.librdf.org/bug370.trig"
	bug370.res
	${CMAKE_CURRENT_SOURCE_DIR}/bug370.out
)

RAPPER_TEST(trig.bug451
	"${RAPPER} -q -i trig -o nquads ${CMAKE_CURRENT_SOURCE_DIR}/bug451.trig http://example.librdf.org/bug451.trig"
	bug451.res
	${CMAKE_CURRENT_SOURCE_DIR}/bug451.out
)

# end raptor/tests/trig/CMakeLists.txt

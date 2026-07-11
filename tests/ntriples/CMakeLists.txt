# raptor/tests/ntriples/CMakeLists.txt
#
# Original listfile by Daniel Richard G. <skunk@iSKUNK.ORG>
# This file is in the public domain.
#

RAPPER_TEST(ntriples.test
	"${RAPPER} -q -i ntriples -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/test.nt http://librdf.org/raptor/tests/test.nt"
	test.res
	${CMAKE_CURRENT_SOURCE_DIR}/test.out
)

RAPPER_EXPECT_FAIL_TEST(ntriples.bad-00
	"${RAPPER} -q -i ntriples -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-00.nt http://librdf.org/raptor/tests/bad-00.nt"
)
RAPPER_EXPECT_FAIL_TEST(ntriples.bad-01
	"${RAPPER} -q -i ntriples -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-01.nt http://librdf.org/raptor/tests/bad-01.nt"
)
RAPPER_EXPECT_FAIL_TEST(ntriples.bad-02
	"${RAPPER} -q -i ntriples -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-02.nt http://librdf.org/raptor/tests/bad-02.nt"
)
RAPPER_EXPECT_FAIL_TEST(ntriples.bad-03
	"${RAPPER} -q -i ntriples -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-03.nt http://librdf.org/raptor/tests/bad-03.nt"
)
RAPPER_EXPECT_FAIL_TEST(ntriples.bad-04
	"${RAPPER} -q -i ntriples -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-04.nt http://librdf.org/raptor/tests/bad-04.nt"
)
RAPPER_EXPECT_FAIL_TEST(ntriples.bad-05
	"${RAPPER} -q -i ntriples -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-05.nt http://librdf.org/raptor/tests/bad-05.nt"
)
RAPPER_EXPECT_FAIL_TEST(ntriples.bad-06
	"${RAPPER} -q -i ntriples -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-06.nt http://librdf.org/raptor/tests/bad-06.nt"
)
RAPPER_EXPECT_FAIL_TEST(ntriples.bad-07
	"${RAPPER} -q -i ntriples -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-07.nt http://librdf.org/raptor/tests/bad-07.nt"
)
RAPPER_EXPECT_FAIL_TEST(ntriples.bad-08
	"${RAPPER} -q -i ntriples -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-08.nt http://librdf.org/raptor/tests/bad-08.nt"
)
RAPPER_EXPECT_FAIL_TEST(ntriples.bad-09
	"${RAPPER} -q -i ntriples -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-09.nt http://librdf.org/raptor/tests/bad-09.nt"
)
RAPPER_EXPECT_FAIL_TEST(ntriples.bad-10
	"${RAPPER} -q -i ntriples -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-10.nt http://librdf.org/raptor/tests/bad-10.nt"
)
RAPPER_EXPECT_FAIL_TEST(ntriples.bad-11
	"${RAPPER} -q -i ntriples -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-11.nt http://librdf.org/raptor/tests/bad-11.nt"
)

RAPPER_TEST(ntriples.testnq-1
	"${RAPPER} -q -i nquads -o nquads file:${CMAKE_CURRENT_SOURCE_DIR}/testnq-1.nq http://librdf.org/raptor/tests/testnq-1.nq"
	testnq-1.res
	${CMAKE_CURRENT_SOURCE_DIR}/testnq-1.out
)

RAPPER_TEST(ntriples.testnq-optional-context
	"${RAPPER} -q -i nquads -o nquads file:${CMAKE_CURRENT_SOURCE_DIR}/testnq-optional-context.nq http://librdf.org/raptor/tests/testnq-optional-context.nq"
	testnq-optional-context.res
	${CMAKE_CURRENT_SOURCE_DIR}/testnq-optional-context.out
)

RAPPER_TEST(ntriples.bug-481
	"${RAPPER} -q -i nquads -o nquads file:${CMAKE_CURRENT_SOURCE_DIR}/bug-481.nq http://librdf.org/raptor/tests/bug-481.nq"
	bug-481.res
	${CMAKE_CURRENT_SOURCE_DIR}/bug-481.out
)

RAPPER_TEST(ntriples.issue-77
	"${RAPPER} -q -i ntriples -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/issue77.nt http://librdf.org/raptor/tests/issue77.nt"
	check-nt.res
	${CMAKE_CURRENT_SOURCE_DIR}/issue77.out
)

# end raptor/tests/ntriples/CMakeLists.txt

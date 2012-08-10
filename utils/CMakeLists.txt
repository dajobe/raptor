# raptor/utils/CMakeLists.txt
#
# Original listfile by Daniel Richard G. <skunk@iSKUNK.ORG>
# This file is in the public domain.
#

INCLUDE_DIRECTORIES(BEFORE
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_BINARY_DIR}/src
	${CMAKE_CURRENT_SOURCE_DIR}
)

IF(NOT HAVE_GETOPT AND NOT HAVE_GETOPT_LONG)
	SET(getopt_sources getopt.c raptor_getopt.h)
ENDIF(NOT HAVE_GETOPT AND NOT HAVE_GETOPT_LONG)

ADD_EXECUTABLE(rapper rapper.c ${getopt_sources})
TARGET_LINK_LIBRARIES(rapper raptor2)

ADD_EXECUTABLE(rdfdiff rdfdiff.c ${getopt_sources})
TARGET_LINK_LIBRARIES(rdfdiff raptor2)

INSTALL(FILES   rapper.1 DESTINATION share/man/man1)
INSTALL(TARGETS rapper   DESTINATION bin)

# end raptor/utils/CMakeLists.txt

# raptor/tests/rdfxml/CMakeLists.txt
#
# Original listfile by Daniel Richard G. <skunk@iSKUNK.ORG>
# This file is in the public domain.
#

RAPPER_TEST(rdfxml.ex-00
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-00.rdf http://librdf.org/raptor/tests/ex-00.rdf"
	ex-00.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-00.out
)

RAPPER_TEST(rdfxml.ex-01
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-01.rdf http://librdf.org/raptor/tests/ex-01.rdf"
	ex-01.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-01.out
)

RAPPER_TEST(rdfxml.ex-02
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-02.rdf http://librdf.org/raptor/tests/ex-02.rdf"
	ex-02.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-02.out
)

RAPPER_TEST(rdfxml.ex-04
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-04.rdf http://librdf.org/raptor/tests/ex-04.rdf"
	ex-04.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-04.out
)

RAPPER_TEST(rdfxml.ex-05
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-05.rdf http://librdf.org/raptor/tests/ex-05.rdf"
	ex-05.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-05.out
)

RAPPER_TEST(rdfxml.ex-06
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-06.rdf http://librdf.org/raptor/tests/ex-06.rdf"
	ex-06.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-06.out
)

RAPPER_TEST(rdfxml.ex-07
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-07.rdf http://librdf.org/raptor/tests/ex-07.rdf"
	ex-07.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-07.out
)

RAPPER_TEST(rdfxml.ex-08
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-08.rdf http://librdf.org/raptor/tests/ex-08.rdf"
	ex-08.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-08.out
)

RAPPER_TEST(rdfxml.ex-09
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-09.rdf http://librdf.org/raptor/tests/ex-09.rdf"
	ex-09.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-09.out
)

RAPPER_TEST(rdfxml.ex-10
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-10.rdf http://librdf.org/raptor/tests/ex-10.rdf"
	ex-10.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-10.out
)

RAPPER_TEST(rdfxml.ex-11
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-11.rdf http://librdf.org/raptor/tests/ex-11.rdf"
	ex-11.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-11.out
)

RAPPER_TEST(rdfxml.ex-12
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-12.rdf http://librdf.org/raptor/tests/ex-12.rdf"
	ex-12.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-12.out
)

RAPPER_TEST(rdfxml.ex-13
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-13.rdf http://librdf.org/raptor/tests/ex-13.rdf"
	ex-13.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-13.out
)

RAPPER_TEST(rdfxml.ex-14
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-14.rdf http://librdf.org/raptor/tests/ex-14.rdf"
	ex-14.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-14.out
)

RAPPER_TEST(rdfxml.ex-15
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-15.rdf http://librdf.org/raptor/tests/ex-15.rdf"
	ex-15.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-15.out
)

RAPPER_TEST(rdfxml.ex-16
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-16.rdf http://librdf.org/raptor/tests/ex-16.rdf"
	ex-16.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-16.out
)

RAPPER_TEST(rdfxml.ex-17
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-17.rdf http://librdf.org/raptor/tests/ex-17.rdf"
	ex-17.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-17.out
)

RAPPER_TEST(rdfxml.ex-18
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-18.rdf http://librdf.org/raptor/tests/ex-18.rdf"
	ex-18.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-18.out
)

RAPPER_TEST(rdfxml.ex-20
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-20.rdf http://librdf.org/raptor/tests/ex-20.rdf"
	ex-20.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-20.out
)

RAPPER_TEST(rdfxml.ex-21
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-21.rdf http://librdf.org/raptor/tests/ex-21.rdf"
	ex-21.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-21.out
)

RAPPER_TEST(rdfxml.ex-22
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-22.rdf http://librdf.org/raptor/tests/ex-22.rdf"
	ex-22.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-22.out
)

RAPPER_TEST(rdfxml.ex-23
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-23.rdf http://librdf.org/raptor/tests/ex-23.rdf"
	ex-23.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-23.out
)

RAPPER_TEST(rdfxml.ex-24
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-24.rdf http://librdf.org/raptor/tests/ex-24.rdf"
	ex-24.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-24.out
)

RAPPER_TEST(rdfxml.ex-25
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-25.rdf http://librdf.org/raptor/tests/ex-25.rdf"
	ex-25.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-25.out
)

RAPPER_TEST(rdfxml.ex-26
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-26.rdf http://librdf.org/raptor/tests/ex-26.rdf"
	ex-26.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-26.out
)

RAPPER_TEST(rdfxml.ex-27
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-27.rdf http://librdf.org/raptor/tests/ex-27.rdf"
	ex-27.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-27.out
)

RAPPER_TEST(rdfxml.ex-28
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-28.rdf http://librdf.org/raptor/tests/ex-28.rdf"
	ex-28.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-28.out
)

RAPPER_TEST(rdfxml.ex-29
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-29.rdf http://librdf.org/raptor/tests/ex-29.rdf"
	ex-29.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-29.out
)

RAPPER_TEST(rdfxml.ex-30
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-30.rdf http://librdf.org/raptor/tests/ex-30.rdf"
	ex-30.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-30.out
)

RAPPER_TEST(rdfxml.ex-31
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-31.rdf http://librdf.org/raptor/tests/ex-31.rdf"
	ex-31.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-31.out
)

RAPPER_TEST(rdfxml.ex-32
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-32.rdf http://librdf.org/raptor/tests/ex-32.rdf"
	ex-32.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-32.out
)

RAPPER_TEST(rdfxml.ex-33
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-33.rdf http://librdf.org/raptor/tests/ex-33.rdf"
	ex-33.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-33.out
)

RAPPER_TEST(rdfxml.ex-34
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-34.rdf http://librdf.org/raptor/tests/ex-34.rdf"
	ex-34.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-34.out
)

RAPPER_TEST(rdfxml.ex-35
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-35.rdf http://librdf.org/raptor/tests/ex-35.rdf"
	ex-35.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-35.out
)

RAPPER_TEST(rdfxml.ex-36
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-36.rdf http://librdf.org/raptor/tests/ex-36.rdf"
	ex-36.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-36.out
)

RAPPER_TEST(rdfxml.ex-37
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-37.rdf http://librdf.org/raptor/tests/ex-37.rdf"
	ex-37.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-37.out
)

RAPPER_TEST(rdfxml.ex-38
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-38.rdf http://librdf.org/raptor/tests/ex-38.rdf"
	ex-38.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-38.out
)

RAPPER_TEST(rdfxml.ex-39
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-39.rdf http://librdf.org/raptor/tests/ex-39.rdf"
	ex-39.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-39.out
)

RAPPER_TEST(rdfxml.ex-40
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-40.rdf http://librdf.org/raptor/tests/ex-40.rdf"
	ex-40.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-40.out
)

RAPPER_TEST(rdfxml.ex-41
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-41.rdf http://librdf.org/raptor/tests/ex-41.rdf"
	ex-41.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-41.out
)

RAPPER_TEST(rdfxml.ex-46
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-46.rdf http://librdf.org/raptor/tests/ex-46.rdf"
	ex-46.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-46.out
)

RAPPER_TEST(rdfxml.ex-47
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-47.rdf http://librdf.org/raptor/tests/ex-47.rdf"
	ex-47.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-47.out
)

RAPPER_TEST(rdfxml.ex-48
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-48.rdf http://librdf.org/raptor/tests/ex-48.rdf"
	ex-48.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-48.out
)

RAPPER_TEST(rdfxml.ex-49
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-49.rdf http://librdf.org/raptor/tests/ex-49.rdf"
	ex-49.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-49.out
)

RAPPER_TEST(rdfxml.ex-51
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-51.rdf http://librdf.org/raptor/tests/ex-51.rdf"
	ex-51.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-51.out
)

RAPPER_TEST(rdfxml.ex-53
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-53.rdf http://librdf.org/raptor/tests/ex-53.rdf"
	ex-53.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-53.out
)

RAPPER_TEST(rdfxml.ex-54
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-54.rdf http://librdf.org/raptor/tests/ex-54.rdf"
	ex-54.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-54.out
)

RAPPER_TEST(rdfxml.ex-56
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-56.rdf http://librdf.org/raptor/tests/ex-56.rdf"
	ex-56.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-56.out
)

RAPPER_TEST(rdfxml.ex-57
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-57.rdf http://librdf.org/raptor/tests/ex-57.rdf"
	ex-57.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-57.out
)

RAPPER_TEST(rdfxml.ex-58
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-58.rdf http://librdf.org/raptor/tests/ex-58.rdf"
	ex-58.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-58.out
)

RAPPER_TEST(rdfxml.ex-61
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-61.rdf http://librdf.org/raptor/tests/ex-61.rdf"
	ex-61.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-61.out
)

RAPPER_TEST(rdfxml.22-rdf-syntax-ns
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/22-rdf-syntax-ns.rdf http://www.w3.org/1999/02/22-rdf-syntax-ns"
	22-rdf-syntax-ns.res
	${CMAKE_CURRENT_SOURCE_DIR}/22-rdf-syntax-ns.out
)

RAPPER_TEST(rdfxml.rdfs-namespace
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/rdfs-namespace.rdf http://www.w3.org/2000/01/rdf-schema"
	rdfs-namespace.res
	${CMAKE_CURRENT_SOURCE_DIR}/rdfs-namespace.out
)

RAPPER_TEST(rdfxml.rdf-schema
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/rdf-schema.rdf http://www.w3.org/2000/01/rdf-schema"
	rdf-schema.res
	${CMAKE_CURRENT_SOURCE_DIR}/rdf-schema.out
)

RAPPER_TEST(rdfxml.owl-schema
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/owl-schema.rdf http://www.w3.org/2002/07/owl"
	owl-schema.res
	${CMAKE_CURRENT_SOURCE_DIR}/owl-schema.out
)

RAPPER_TEST(rdfxml.daml-oil
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/daml-oil.rdf http://www.daml.org/2001/03/daml+oil"
	daml-oil.res
	${CMAKE_CURRENT_SOURCE_DIR}/daml-oil.out
)

RAPPER_TEST(rdfxml.wine
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/wine.rdf http://www.w3.org/TR/owl-guide/wine.rdf"
	wine.res
	${CMAKE_CURRENT_SOURCE_DIR}/wine.out
)

RAPPER_TEST(rdfxml.ex-55
	"${RAPPER} -q -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/ex-55.rdf http://librdf.org/raptor/tests/ex-55.rdf"
	ex-55.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-55.out
)

RAPPER_TEST(rdfxml.ex-19
	"${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/ex-19.rdf http://librdf.org/raptor/tests/ex-19.rdf"
	ex-19.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-19.out
)

RAPPER_TEST(rdfxml.ex-52
	"${RAPPER} -q -f scanForRDF -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/ex-52.svg http://librdf.org/raptor/tests/ex-52.rdf"
	ex-52.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-52.out
)

ADD_TEST(rdfxml.bad-00 ${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-00.rdf http://librdf.org/raptor/tests/bad-00.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-01 ${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-01.rdf http://librdf.org/raptor/tests/bad-01.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-02 ${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-02.rdf http://librdf.org/raptor/tests/bad-02.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-03 ${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-03.rdf http://librdf.org/raptor/tests/bad-03.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-04 ${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-04.rdf http://librdf.org/raptor/tests/bad-04.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-05 ${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-05.rdf http://librdf.org/raptor/tests/bad-05.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-06 ${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-06.rdf http://librdf.org/raptor/tests/bad-06.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-07 ${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-07.rdf http://librdf.org/raptor/tests/bad-07.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-08 ${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-08.rdf http://librdf.org/raptor/tests/bad-08.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-09 ${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-09.rdf http://librdf.org/raptor/tests/bad-09.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-10 ${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-10.rdf http://librdf.org/raptor/tests/bad-10.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-11 ${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-11.rdf http://librdf.org/raptor/tests/bad-11.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-12 ${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-12.rdf http://librdf.org/raptor/tests/bad-12.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-13 ${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-13.rdf http://librdf.org/raptor/tests/bad-13.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-14 ${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-14.rdf http://librdf.org/raptor/tests/bad-14.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-18 ${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-18.rdf http://librdf.org/raptor/tests/bad-18.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-19 ${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-19.rdf http://librdf.org/raptor/tests/bad-19.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-20 ${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-20.rdf http://librdf.org/raptor/tests/bad-20.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-21 ${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-21.rdf http://librdf.org/raptor/tests/bad-21.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-22 ${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-22.rdf http://librdf.org/raptor/tests/bad-22.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-23 ${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-23.rdf http://librdf.org/raptor/tests/bad-23.rdf) # WILL_FAIL

SET_TESTS_PROPERTIES(
	rdfxml.bad-00
	rdfxml.bad-01
	rdfxml.bad-02
	rdfxml.bad-03
	rdfxml.bad-04
	rdfxml.bad-05
	rdfxml.bad-06
	rdfxml.bad-07
	rdfxml.bad-08
	rdfxml.bad-09
	rdfxml.bad-10
	rdfxml.bad-11
	rdfxml.bad-12
	rdfxml.bad-13
	rdfxml.bad-14
	rdfxml.bad-18
	rdfxml.bad-19
	rdfxml.bad-20
	rdfxml.bad-21
	rdfxml.bad-22
	rdfxml.bad-23
	PROPERTIES
	WILL_FAIL TRUE
)

ADD_TEST(rdfxml.bad-15 ${RAPPER} -q -m strict -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-15.rdf http://librdf.org/raptor/tests/bad-15.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-16 ${RAPPER} -q -m strict -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-16.rdf http://librdf.org/raptor/tests/bad-16.rdf) # WILL_FAIL
ADD_TEST(rdfxml.bad-17 ${RAPPER} -q -m strict -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/bad-17.rdf http://librdf.org/raptor/tests/bad-17.rdf) # WILL_FAIL

SET_TESTS_PROPERTIES(
	rdfxml.bad-15
	rdfxml.bad-16
	rdfxml.bad-17
	PROPERTIES
	WILL_FAIL TRUE
)

RAPPER_TEST(rdfxml.warn-00
	"${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/warn-00.rdf http://librdf.org/raptor/tests/warn-00.rdf"
	warn-00.res
	${CMAKE_CURRENT_SOURCE_DIR}/warn-00.out
)

RAPPER_TEST(rdfxml.warn-02
	"${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/warn-02.rdf http://librdf.org/raptor/tests/warn-02.rdf"
	warn-02.res
	${CMAKE_CURRENT_SOURCE_DIR}/warn-02.out
)

RAPPER_TEST(rdfxml.warn-04
	"${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/warn-04.rdf http://librdf.org/raptor/tests/warn-04.rdf"
	warn-04.res
	${CMAKE_CURRENT_SOURCE_DIR}/warn-04.out
)

RAPPER_TEST(rdfxml.warn-05
	"${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/warn-05.rdf http://librdf.org/raptor/tests/warn-05.rdf"
	warn-05.res
	${CMAKE_CURRENT_SOURCE_DIR}/warn-05.out
)

RAPPER_TEST(rdfxml.warn-06
	"${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/warn-06.rdf http://librdf.org/raptor/tests/warn-06.rdf"
	warn-06.res
	${CMAKE_CURRENT_SOURCE_DIR}/warn-06.out
)

RAPPER_TEST(rdfxml.warn-07
	"${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/warn-07.rdf http://librdf.org/raptor/tests/warn-07.rdf"
	warn-07.res
	${CMAKE_CURRENT_SOURCE_DIR}/warn-07.out
)

RAPPER_TEST(rdfxml.ex-03
	"${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/ex-03.rdf http://librdf.org/raptor/tests/ex-03.rdf"
	ex-03.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-03.out
)

RAPPER_TEST(rdfxml.ex-42
	"${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/ex-42.rdf http://librdf.org/raptor/tests/ex-42.rdf"
	ex-42.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-42.out
)

RAPPER_TEST(rdfxml.ex-43
	"${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/ex-43.rdf http://librdf.org/raptor/tests/ex-43.rdf"
	ex-43.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-43.out
)

RAPPER_TEST(rdfxml.ex-44
	"${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/ex-44.rdf http://librdf.org/raptor/tests/ex-44.rdf"
	ex-44.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-44.out
)

RAPPER_TEST(rdfxml.ex-45
	"${RAPPER} -q -o ntriples file:${CMAKE_CURRENT_SOURCE_DIR}/ex-45.rdf http://librdf.org/raptor/tests/ex-45.rdf"
	ex-45.res
	${CMAKE_CURRENT_SOURCE_DIR}/ex-45.out
)

RAPPER_TEST(rdfxml.ex-59
	"${RAPPER} -q -i ntriples -o rdfxml ${CMAKE_CURRENT_SOURCE_DIR}/ex-59.nt -"
	ex-59-rdfxml.rdf
	${CMAKE_CURRENT_SOURCE_DIR}/ex-59.rdf
)

RAPPER_TEST(rdfxml.ex-60
	"${RAPPER} -q -i ntriples -o rdfxml ${CMAKE_CURRENT_SOURCE_DIR}/ex-60.nt -"
	ex-60-rdfxml.rdf
	${CMAKE_CURRENT_SOURCE_DIR}/ex-60.rdf
)

# end raptor/tests/rdfxml/CMakeLists.txt

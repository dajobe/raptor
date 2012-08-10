# raptor/tests/feeds/CMakeLists.txt
#
# Original listfile by Daniel Richard G. <skunk@iSKUNK.ORG>
# This file is in the public domain.
#

IF(RAPTOR_PARSER_RDFXML)

	RAPPER_TEST(feeds.test01.rdf
		"${RAPPER} -q -i rdfxml -o turtle -f writeBaseURI=0 -O http://example.org/whatsnew file:${CMAKE_CURRENT_SOURCE_DIR}/test01.rdf"
		test01.ttl
		${CMAKE_CURRENT_SOURCE_DIR}/test01-result.ttl
	)

	RAPPER_TEST(feeds.test02.rdf
		"${RAPPER} -q -i rdfxml -o turtle -f writeBaseURI=0 -O http://example.org/news/ file:${CMAKE_CURRENT_SOURCE_DIR}/test02.rdf"
		test02.ttl
		${CMAKE_CURRENT_SOURCE_DIR}/test02-result.ttl
	)

	RAPPER_TEST(feeds.test03.rdf
		"${RAPPER} -q -i rdfxml -o turtle -f writeBaseURI=0 -O http://example.org file:${CMAKE_CURRENT_SOURCE_DIR}/test03.rdf"
		test03.ttl
		${CMAKE_CURRENT_SOURCE_DIR}/test03-result.ttl
	)

ENDIF(RAPTOR_PARSER_RDFXML)

IF(RAPTOR_PARSER_RSS)

	RAPPER_TEST(feeds.test04.atom
		"${RAPPER} -q -i rss-tag-soup -o turtle -f writeBaseURI=0 -O http://www.example.org/blog/ file:${CMAKE_CURRENT_SOURCE_DIR}/test04.atom"
		test04.ttl
		${CMAKE_CURRENT_SOURCE_DIR}/test04-result.ttl
	)

	RAPPER_TEST(feeds.test05.atom
		"${RAPPER} -q -i rss-tag-soup -o turtle -f writeBaseURI=0 -O http://www.example.org/blog/ file:${CMAKE_CURRENT_SOURCE_DIR}/test05.atom"
		test05.ttl
		${CMAKE_CURRENT_SOURCE_DIR}/test05-result.ttl
	)

ENDIF(RAPTOR_PARSER_RSS)

IF(RAPTOR_SERIALIZER_ATOM)

	RAPPER_TEST(feeds.test01.ttl
		"${RAPPER} -q -i turtle -o atom -f writeBaseURI=0 -f rssTriples=atom-triples -f atomEntryUri=http://example.org/2006/03/28/blog-item -I http://example.org/whatsnew -O http://example.org/whatsnew file:test01.ttl"
		test01.atom
		${CMAKE_CURRENT_SOURCE_DIR}/test01-result.atom
	)

	RAPPER_TEST(feeds.test02.ttl
		"${RAPPER} -q -i turtle -o atom -f writeBaseURI=0 -f rssTriples=atom-triples -I http://example.org/news/ -O http://example.org/news/ file:test02.ttl"
		test02.atom
		${CMAKE_CURRENT_SOURCE_DIR}/test02-result.atom
	)

	RAPPER_TEST(feeds.test03.ttl
		"${RAPPER} -q -i turtle -o atom -f writeBaseURI=0 -f rssTriples=atom-triples -I http://example.org -O http://example.org file:test03.ttl"
		test03.atom
		${CMAKE_CURRENT_SOURCE_DIR}/test03-result.atom
	)

ENDIF(RAPTOR_SERIALIZER_ATOM)

IF(HAVE_JING)
	ADD_TEST(feeds.jing-test01.atom ${JING} ${CMAKE_CURRENT_SOURCE_DIR}/atom.rng test01.atom)
	ADD_TEST(feeds.jing-test02.atom ${JING} ${CMAKE_CURRENT_SOURCE_DIR}/atom.rng test02.atom)
	ADD_TEST(feeds.jing-test03.atom ${JING} ${CMAKE_CURRENT_SOURCE_DIR}/atom.rng test03.atom)
	ADD_TEST(feeds.jing-test04.atom ${JING} ${CMAKE_CURRENT_SOURCE_DIR}/atom.rng ${CMAKE_CURRENT_SOURCE_DIR}/test04.atom)
	ADD_TEST(feeds.jing-test05.atom ${JING} ${CMAKE_CURRENT_SOURCE_DIR}/atom.rng ${CMAKE_CURRENT_SOURCE_DIR}/test05.atom)
ENDIF(HAVE_JING)

# end raptor/tests/feeds/CMakeLists.txt

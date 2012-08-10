# raptor/tests/rdfa11/CMakeLists.txt
#
# Original listfile by Daniel Richard G. <skunk@iSKUNK.ORG>
# This file is in the public domain.
#

IF(RAPTOR_PARSER_RDFA)

	RAPPER_RDFDIFF_TEST(rdfa11.0001
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0001.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0001.xml"
		0001-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0001.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0001.out 0001-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0006
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0006.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0006.xml"
		0006-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0006.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0006.out 0006-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0007
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0007.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0007.xml"
		0007-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0007.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0007.out 0007-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0008
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0008.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0008.xml"
		0008-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0008.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0008.out 0008-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0009
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0009.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0009.xml"
		0009-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0009.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0009.out 0009-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0010
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0010.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0010.xml"
		0010-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0010.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0010.out 0010-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0012
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0012.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0012.xml"
		0012-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0012.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0012.out 0012-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0013
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0013.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0013.xml"
		0013-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0013.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0013.out 0013-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0014
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0014.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0014.xml"
		0014-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0014.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0014.out 0014-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0015
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0015.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0015.xml"
		0015-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0015.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0015.out 0015-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0017
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0017.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0017.xml"
		0017-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0017.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0017.out 0017-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0018
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0018.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0018.xml"
		0018-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0018.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0018.out 0018-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0019
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0019.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0019.xml"
		0019-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0019.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0019.out 0019-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0020
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0020.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0020.xml"
		0020-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0020.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0020.out 0020-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0021
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0021.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0021.xml"
		0021-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0021.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0021.out 0021-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0023
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0023.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0023.xml"
		0023-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0023.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0023.out 0023-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0025
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0025.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0025.xml"
		0025-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0025.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0025.out 0025-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0026
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0026.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0026.xml"
		0026-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0026.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0026.out 0026-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0027
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0027.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0027.xml"
		0027-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0027.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0027.out 0027-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0029
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0029.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0029.xml"
		0029-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0029.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0029.out 0029-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0030
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0030.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0030.xml"
		0030-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0030.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0030.out 0030-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0031
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0031.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0031.xml"
		0031-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0031.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0031.out 0031-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0032
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0032.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0032.xml"
		0032-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0032.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0032.out 0032-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0033
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0033.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0033.xml"
		0033-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0033.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0033.out 0033-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0034
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0034.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0034.xml"
		0034-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0034.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0034.out 0034-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0035
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0035.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0035.xml"
		0035-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0035.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0035.out 0035-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0036
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0036.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0036.xml"
		0036-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0036.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0036.out 0036-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0037
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0037.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0037.xml"
		0037-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0037.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0037.out 0037-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0038
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0038.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0038.xml"
		0038-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0038.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0038.out 0038-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0039
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0039.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0039.xml"
		0039-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0039.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0039.out 0039-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0041
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0041.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0041.xml"
		0041-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0041.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0041.out 0041-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0048
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0048.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0048.xml"
		0048-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0048.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0048.out 0048-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0049
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0049.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0049.xml"
		0049-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0049.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0049.out 0049-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0050
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0050.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0050.xml"
		0050-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0050.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0050.out 0050-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0051
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0051.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0051.xml"
		0051-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0051.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0051.out 0051-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0052
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0052.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0052.xml"
		0052-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0052.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0052.out 0052-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0053
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0053.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0053.xml"
		0053-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0053.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0053.out 0053-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0054
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0054.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0054.xml"
		0054-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0054.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0054.out 0054-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0055
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0055.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0055.xml"
		0055-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0055.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0055.out 0055-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0056
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0056.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0056.xml"
		0056-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0056.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0056.out 0056-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0057
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0057.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0057.xml"
		0057-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0057.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0057.out 0057-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0059
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0059.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0059.xml"
		0059-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0059.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0059.out 0059-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0060
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0060.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0060.xml"
		0060-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0060.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0060.out 0060-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0061
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0061.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0061.xml"
		0061-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0061.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0061.out 0061-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0062
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0062.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0062.xml"
		0062-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0062.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0062.out 0062-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0063
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0063.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0063.xml"
		0063-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0063.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0063.out 0063-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0064
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0064.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0064.xml"
		0064-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0064.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0064.out 0064-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0065
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0065.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0065.xml"
		0065-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0065.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0065.out 0065-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0066
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0066.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0066.xml"
		0066-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0066.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0066.out 0066-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0067
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0067.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0067.xml"
		0067-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0067.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0067.out 0067-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0068
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0068.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0068.xml"
		0068-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0068.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0068.out 0068-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0069
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0069.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0069.xml"
		0069-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0069.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0069.out 0069-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0070
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0070.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0070.xml"
		0070-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0070.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0070.out 0070-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0071
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0071.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0071.xml"
		0071-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0071.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0071.out 0071-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0072
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0072.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0072.xml"
		0072-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0072.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0072.out 0072-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0073
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0073.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0073.xml"
		0073-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0073.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0073.out 0073-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0074
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0074.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0074.xml"
		0074-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0074.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0074.out 0074-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0075
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0075.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0075.xml"
		0075-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0075.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0075.out 0075-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0076
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0076.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0076.xml"
		0076-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0076.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0076.out 0076-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0077
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0077.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0077.xml"
		0077-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0077.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0077.out 0077-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0079
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0079.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0079.xml"
		0079-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0079.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0079.out 0079-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0080
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0080.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0080.xml"
		0080-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0080.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0080.out 0080-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0083
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0083.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0083.xml"
		0083-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0083.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0083.out 0083-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0084
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0084.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0084.xml"
		0084-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0084.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0084.out 0084-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0085
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0085.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0085.xml"
		0085-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0085.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0085.out 0085-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0087
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0087.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0087.xml"
		0087-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0087.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0087.out 0087-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0088
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0088.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0088.xml"
		0088-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0088.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0088.out 0088-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0089
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0089.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0089.xml"
		0089-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0089.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0089.out 0089-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0091
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0091.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0091.xml"
		0091-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0091.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0091.out 0091-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0093
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0093.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0093.xml"
		0093-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0093.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0093.out 0093-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0099
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0099.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0099.xml"
		0099-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0099.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0099.out 0099-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0104
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0104.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0104.xml"
		0104-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0104.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0104.out 0104-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0106
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0106.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0106.xml"
		0106-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0106.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0106.out 0106-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0107
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0107.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0107.xml"
		0107-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0107.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0107.out 0107-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0108
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0108.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0108.xml"
		0108-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0108.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0108.out 0108-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0109
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0109.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0109.xml"
		0109-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0109.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0109.out 0109-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0110
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0110.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0110.xml"
		0110-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0110.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0110.out 0110-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0111
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0111.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0111.xml"
		0111-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0111.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0111.out 0111-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0112
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0112.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0112.xml"
		0112-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0112.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0112.out 0112-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0113
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0113.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0113.xml"
		0113-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0113.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0113.out 0113-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0114
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0114.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0114.xml"
		0114-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0114.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0114.out 0114-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0115
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0115.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0115.xml"
		0115-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0115.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0115.out 0115-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0117
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0117.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0117.xml"
		0117-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0117.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0117.out 0117-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0118
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0118.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0118.xml"
		0118-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0118.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0118.out 0118-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0119
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0119.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0119.xml"
		0119-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0119.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0119.out 0119-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0120
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0120.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0120.xml"
		0120-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0120.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0120.out 0120-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0121
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0121.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0121.xml"
		0121-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0121.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0121.out 0121-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0122
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0122.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0122.xml"
		0122-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0122.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0122.out 0122-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0126
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0126.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0126.xml"
		0126-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0126.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0126.out 0126-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0131
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0131.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0131.xml"
		0131-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0131.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0131.out 0131-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0134
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0134.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0134.xml"
		0134-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0134.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0134.out 0134-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0140
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0140.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0140.xml"
		0140-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0140.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0140.out 0140-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0147
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0147.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0147.xml"
		0147-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0147.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0147.out 0147-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0172
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0172.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0172.xml"
		0172-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0172.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0172.out 0172-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0173
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0173.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0173.xml"
		0173-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0173.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0173.out 0173-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0174
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0174.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0174.xml"
		0174-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0174.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0174.out 0174-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0175
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0175.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0175.xml"
		0175-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0175.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0175.out 0175-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0176
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0176.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0176.xml"
		0176-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0176.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0176.out 0176-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0177
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0177.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0177.xml"
		0177-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0177.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0177.out 0177-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0178
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0178.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0178.xml"
		0178-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0178.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0178.out 0178-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0179
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0179.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0179.xml"
		0179-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0179.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0179.out 0179-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0180
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0180.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0180.xml"
		0180-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0180.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0180.out 0180-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0181
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0181.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0181.xml"
		0181-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0181.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0181.out 0181-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0182
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0182.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0182.xml"
		0182-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0182.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0182.out 0182-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0183
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0183.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0183.xml"
		0183-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0183.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0183.out 0183-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0186
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0186.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0186.xml"
		0186-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0186.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0186.out 0186-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0187
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0187.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0187.xml"
		0187-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0187.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0187.out 0187-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0188
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0188.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0188.xml"
		0188-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0188.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0188.out 0188-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0189
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0189.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0189.xml"
		0189-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0189.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0189.out 0189-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0190
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0190.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0190.xml"
		0190-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0190.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0190.out 0190-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0196
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0196.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0196.xml"
		0196-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0196.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0196.out 0196-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0197
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0197.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0197.xml"
		0197-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0197.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0197.out 0197-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0198
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0198.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0198.xml"
		0198-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0198.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0198.out 0198-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0201
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0201.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0201.xml"
		0201-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0201.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0201.out 0201-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0202
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0202.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0202.xml"
		0202-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0202.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0202.out 0202-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0203
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0203.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0203.xml"
		0203-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0203.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0203.out 0203-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0206
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0206.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0206.xml"
		0206-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0206.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0206.out 0206-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0207
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0207.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0207.xml"
		0207-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0207.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0207.out 0207-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0213
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0213.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0213.xml"
		0213-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0213.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0213.out 0213-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0214
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0214.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0214.xml"
		0214-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0214.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0214.out 0214-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0216
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0216.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0216.xml"
		0216-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0216.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0216.out 0216-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0217
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0217.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0217.xml"
		0217-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0217.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0217.out 0217-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0218
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0218.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0218.xml"
		0218-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0218.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0218.out 0218-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0219
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0219.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0219.xml"
		0219-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0219.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0219.out 0219-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0220
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0220.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0220.xml"
		0220-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0220.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0220.out 0220-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0221
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0221.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0221.xml"
		0221-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0221.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0221.out 0221-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0222
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0222.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0222.xml"
		0222-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0222.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0222.out 0222-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0223
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0223.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0223.xml"
		0223-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0223.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0223.out 0223-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0224
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0224.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0224.xml"
		0224-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0224.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0224.out 0224-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0225
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0225.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0225.xml"
		0225-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0225.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0225.out 0225-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0226
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0226.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0226.xml"
		0226-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0226.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0226.out 0226-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0227
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0227.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0227.xml"
		0227-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0227.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0227.out 0227-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0228
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0228.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0228.xml"
		0228-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0228.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0228.out 0228-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0229
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0229.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0229.xml"
		0229-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0229.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0229.out 0229-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0230
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0230.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0230.xml"
		0230-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0230.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0230.out 0230-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0231
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0231.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0231.xml"
		0231-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0231.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0231.out 0231-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0232
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0232.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0232.xml"
		0232-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0232.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0232.out 0232-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0233
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0233.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0233.xml"
		0233-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0233.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0233.out 0233-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0234
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0234.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0234.xml"
		0234-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0234.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0234.out 0234-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0235
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0235.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0235.xml"
		0235-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0235.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0235.out 0235-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0236
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0236.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0236.xml"
		0236-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0236.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0236.out 0236-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0237
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0237.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0237.xml"
		0237-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0237.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0237.out 0237-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0238
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0238.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0238.xml"
		0238-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0238.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0238.out 0238-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0239
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0239.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0239.xml"
		0239-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0239.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0239.out 0239-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0246
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0246.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0246.xml"
		0246-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0246.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0246.out 0246-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0247
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0247.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0247.xml"
		0247-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0247.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0247.out 0247-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0248
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0248.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0248.xml"
		0248-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0248.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0248.out 0248-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0249
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0249.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0249.xml"
		0249-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0249.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0249.out 0249-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0250
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0250.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0250.xml"
		0250-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0250.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0250.out 0250-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0251
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0251.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0251.xml"
		0251-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0251.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0251.out 0251-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0252
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0252.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0252.xml"
		0252-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0252.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0252.out 0252-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0253
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0253.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0253.xml"
		0253-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0253.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0253.out 0253-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0254
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0254.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0254.xml"
		0254-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0254.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0254.out 0254-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0255
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0255.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0255.xml"
		0255-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0255.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0255.out 0255-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0256
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0256.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0256.xml"
		0256-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0256.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0256.out 0256-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0257
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0257.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0257.xml"
		0257-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0257.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0257.out 0257-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0258
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0258.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0258.xml"
		0258-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0258.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0258.out 0258-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0259
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0259.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0259.xml"
		0259-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0259.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0259.out 0259-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0260
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0260.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0260.xml"
		0260-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0260.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0260.out 0260-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0261
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0261.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0261.xml"
		0261-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0261.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0261.out 0261-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0262
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0262.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0262.xml"
		0262-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0262.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0262.out 0262-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0263
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0263.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0263.xml"
		0263-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0263.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0263.out 0263-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0264
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0264.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0264.xml"
		0264-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0264.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0264.out 0264-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0265
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0265.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0265.xml"
		0265-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0265.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0265.out 0265-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0266
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0266.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0266.xml"
		0266-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0266.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0266.out 0266-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0267
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0267.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0267.xml"
		0267-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0267.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0267.out 0267-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0268
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0268.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0268.xml"
		0268-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0268.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0268.out 0268-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0269
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0269.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0269.xml"
		0269-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0269.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0269.out 0269-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0269
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0269.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0269.xml"
		0269-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0269.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0269.out 0269-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0271
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0271.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0271.xml"
		0271-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0271.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0271.out 0271-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0272
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0272.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0272.xml"
		0272-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0272.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0272.out 0272-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0273
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0273.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0273.xml"
		0273-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0273.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0273.out 0273-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0274
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0274.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0274.xml"
		0274-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0274.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0274.out 0274-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0275
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0275.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0275.xml"
		0275-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0275.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0275.out 0275-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0276
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0276.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0276.xml"
		0276-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0276.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0276.out 0276-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0277
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0277.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0277.xml"
		0277-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0277.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0277.out 0277-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0278
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0278.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0278.xml"
		0278-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0278.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0278.out 0278-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0279
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0279.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0279.xml"
		0279-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0279.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0279.out 0279-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0280
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0280.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0280.xml"
		0280-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0280.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0280.out 0280-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0281
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0281.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0281.xml"
		0281-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0281.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0281.out 0281-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0282
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0282.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0282.xml"
		0282-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0282.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0282.out 0282-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0283
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0283.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0283.xml"
		0283-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0283.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0283.out 0283-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0284
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0284.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0284.xml"
		0284-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0284.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0284.out 0284-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0285
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0285.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0285.xml"
		0285-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0285.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0285.out 0285-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0286
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0286.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0286.xml"
		0286-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0286.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0286.out 0286-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0287
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0287.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0287.xml"
		0287-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0287.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0287.out 0287-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0289
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0289.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0289.xml"
		0289-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0289.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0289.out 0289-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0290
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0290.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0290.xml"
		0290-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0290.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0290.out 0290-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0291
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0291.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0291.xml"
		0291-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0291.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0291.out 0291-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0292
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0292.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0292.xml"
		0292-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0292.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0292.out 0292-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0293
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0293.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0293.xml"
		0293-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0293.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0293.out 0293-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0295
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0295.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0295.xml"
		0295-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0295.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0295.out 0295-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0296
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0296.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0296.xml"
		0296-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0296.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0296.out 0296-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0297
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0297.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0297.xml"
		0297-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0297.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0297.out 0297-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0298
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0298.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0298.xml"
		0298-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0298.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0298.out 0298-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0299
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0299.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0299.xml"
		0299-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0299.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0299.out 0299-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0300
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0300.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0300.xml"
		0300-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0300.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0300.out 0300-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0301
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0301.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0301.xml"
		0301-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0301.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0301.out 0301-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0302
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0302.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0302.xml"
		0302-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0302.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0302.out 0302-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0303
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0303.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0303.xml"
		0303-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0303.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0303.out 0303-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa11.0304
		"${RAPPER} -f noNet -q -i rdfa11 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0304.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0304.xml"
		0304-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0304.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0304.out 0304-res.nt"
	)

ENDIF(RAPTOR_PARSER_RDFA)

# end raptor/tests/rdfa11/CMakeLists.txt

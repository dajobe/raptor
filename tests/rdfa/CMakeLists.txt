# raptor/tests/rdfa/CMakeLists.txt
#
# Original listfile by Daniel Richard G. <skunk@iSKUNK.ORG>
# This file is in the public domain.
#

IF(RAPTOR_PARSER_RDFA)

	RAPPER_RDFDIFF_TEST(rdfa.0001
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0001.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0001.xml"
		0001-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0001.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0001.out 0001-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0006
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0006.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0006.xml"
		0006-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0006.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0006.out 0006-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0007
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0007.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0007.xml"
		0007-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0007.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0007.out 0007-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0008
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0008.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0008.xml"
		0008-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0008.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0008.out 0008-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0009
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0009.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0009.xml"
		0009-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0009.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0009.out 0009-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0010
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0010.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0010.xml"
		0010-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0010.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0010.out 0010-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0012
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0012.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0012.xml"
		0012-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0012.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0012.out 0012-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0013
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0013.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0013.xml"
		0013-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0013.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0013.out 0013-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0014
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0014.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0014.xml"
		0014-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0014.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0014.out 0014-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0015
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0015.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0015.xml"
		0015-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0015.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0015.out 0015-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0017
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0017.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0017.xml"
		0017-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0017.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0017.out 0017-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0018
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0018.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0018.xml"
		0018-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0018.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0018.out 0018-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0019
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0019.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0019.xml"
		0019-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0019.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0019.out 0019-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0020
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0020.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0020.xml"
		0020-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0020.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0020.out 0020-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0021
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0021.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0021.xml"
		0021-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0021.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0021.out 0021-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0023
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0023.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0023.xml"
		0023-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0023.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0023.out 0023-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0025
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0025.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0025.xml"
		0025-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0025.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0025.out 0025-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0026
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0026.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0026.xml"
		0026-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0026.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0026.out 0026-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0027
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0027.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0027.xml"
		0027-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0027.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0027.out 0027-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0029
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0029.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0029.xml"
		0029-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0029.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0029.out 0029-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0030
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0030.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0030.xml"
		0030-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0030.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0030.out 0030-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0031
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0031.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0031.xml"
		0031-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0031.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0031.out 0031-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0032
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0032.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0032.xml"
		0032-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0032.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0032.out 0032-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0033
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0033.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0033.xml"
		0033-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0033.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0033.out 0033-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0034
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0034.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0034.xml"
		0034-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0034.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0034.out 0034-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0035
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0035.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0035.xml"
		0035-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0035.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0035.out 0035-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0036
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0036.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0036.xml"
		0036-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0036.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0036.out 0036-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0037
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0037.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0037.xml"
		0037-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0037.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0037.out 0037-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0038
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0038.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0038.xml"
		0038-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0038.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0038.out 0038-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0039
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0039.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0039.xml"
		0039-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0039.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0039.out 0039-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0040
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0040.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0040.xml"
		0040-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0040.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0040.out 0040-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0041
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0041.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0041.xml"
		0041-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0041.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0041.out 0041-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0042
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0042.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0042.xml"
		0042-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0042.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0042.out 0042-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0046
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0046.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0046.xml"
		0046-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0046.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0046.out 0046-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0047
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0047.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0047.xml"
		0047-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0047.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0047.out 0047-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0048
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0048.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0048.xml"
		0048-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0048.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0048.out 0048-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0049
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0049.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0049.xml"
		0049-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0049.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0049.out 0049-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0050
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0050.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0050.xml"
		0050-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0050.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0050.out 0050-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0051
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0051.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0051.xml"
		0051-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0051.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0051.out 0051-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0052
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0052.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0052.xml"
		0052-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0052.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0052.out 0052-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0053
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0053.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0053.xml"
		0053-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0053.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0053.out 0053-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0054
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0054.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0054.xml"
		0054-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0054.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0054.out 0054-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0055
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0055.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0055.xml"
		0055-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0055.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0055.out 0055-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0056
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0056.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0056.xml"
		0056-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0056.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0056.out 0056-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0057
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0057.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0057.xml"
		0057-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0057.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0057.out 0057-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0058
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0058.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0058.xml"
		0058-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0058.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0058.out 0058-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0059
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0059.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0059.xml"
		0059-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0059.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0059.out 0059-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0060
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0060.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0060.xml"
		0060-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0060.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0060.out 0060-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0061
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0061.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0061.xml"
		0061-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0061.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0061.out 0061-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0062
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0062.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0062.xml"
		0062-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0062.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0062.out 0062-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0063
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0063.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0063.xml"
		0063-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0063.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0063.out 0063-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0064
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0064.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0064.xml"
		0064-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0064.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0064.out 0064-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0065
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0065.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0065.xml"
		0065-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0065.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0065.out 0065-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0066
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0066.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0066.xml"
		0066-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0066.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0066.out 0066-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0067
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0067.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0067.xml"
		0067-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0067.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0067.out 0067-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0068
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0068.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0068.xml"
		0068-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0068.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0068.out 0068-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0069
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0069.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0069.xml"
		0069-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0069.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0069.out 0069-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0070
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0070.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0070.xml"
		0070-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0070.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0070.out 0070-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0071
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0071.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0071.xml"
		0071-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0071.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0071.out 0071-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0072
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0072.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0072.xml"
		0072-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0072.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0072.out 0072-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0073
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0073.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0073.xml"
		0073-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0073.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0073.out 0073-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0074
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0074.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0074.xml"
		0074-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0074.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0074.out 0074-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0075
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0075.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0075.xml"
		0075-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0075.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0075.out 0075-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0076
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0076.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0076.xml"
		0076-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0076.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0076.out 0076-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0077
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0077.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0077.xml"
		0077-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0077.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0077.out 0077-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0078
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0078.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0078.xml"
		0078-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0078.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0078.out 0078-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0079
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0079.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0079.xml"
		0079-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0079.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0079.out 0079-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0080
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0080.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0080.xml"
		0080-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0080.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0080.out 0080-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0081
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0081.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0081.xml"
		0081-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0081.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0081.out 0081-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0082
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0082.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0082.xml"
		0082-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0082.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0082.out 0082-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0083
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0083.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0083.xml"
		0083-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0083.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0083.out 0083-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0084
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0084.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0084.xml"
		0084-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0084.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0084.out 0084-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0085
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0085.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0085.xml"
		0085-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0085.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0085.out 0085-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0087
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0087.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0087.xml"
		0087-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0087.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0087.out 0087-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0088
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0088.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0088.xml"
		0088-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0088.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0088.out 0088-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0089
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0089.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0089.xml"
		0089-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0089.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0089.out 0089-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0090
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0090.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0090.xml"
		0090-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0090.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0090.out 0090-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0091
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0091.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0091.xml"
		0091-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0091.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0091.out 0091-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0093
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0093.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0093.xml"
		0093-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0093.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0093.out 0093-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0099
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0099.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0099.xml"
		0099-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0099.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0099.out 0099-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0104
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0104.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0104.xml"
		0104-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0104.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0104.out 0104-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0106
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0106.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0106.xml"
		0106-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0106.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0106.out 0106-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0107
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0107.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0107.xml"
		0107-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0107.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0107.out 0107-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0108
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0108.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0108.xml"
		0108-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0108.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0108.out 0108-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0109
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0109.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0109.xml"
		0109-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0109.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0109.out 0109-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0110
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0110.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0110.xml"
		0110-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0110.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0110.out 0110-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0111
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0111.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0111.xml"
		0111-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0111.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0111.out 0111-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0112
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0112.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0112.xml"
		0112-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0112.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0112.out 0112-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0113
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0113.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0113.xml"
		0113-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0113.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0113.out 0113-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0114
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0114.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0114.xml"
		0114-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0114.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0114.out 0114-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0115
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0115.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0115.xml"
		0115-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0115.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0115.out 0115-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0117
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0117.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0117.xml"
		0117-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0117.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0117.out 0117-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0118
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0118.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0118.xml"
		0118-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0118.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0118.out 0118-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0119
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0119.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0119.xml"
		0119-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0119.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0119.out 0119-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0120
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0120.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0120.xml"
		0120-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0120.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0120.out 0120-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0121
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0121.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0121.xml"
		0121-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0121.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0121.out 0121-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0122
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0122.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0122.xml"
		0122-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0122.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0122.out 0122-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0126
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0126.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0126.xml"
		0126-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0126.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0126.out 0126-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0131
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0131.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0131.xml"
		0131-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0131.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0131.out 0131-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0134
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0134.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0134.xml"
		0134-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0134.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0134.out 0134-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0140
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0140.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0140.xml"
		0140-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0140.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0140.out 0140-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0147
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0147.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0147.xml"
		0147-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0147.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0147.out 0147-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0172
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0172.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0172.xml"
		0172-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0172.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0172.out 0172-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0173
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0173.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0173.xml"
		0173-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0173.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0173.out 0173-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0174
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0174.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0174.xml"
		0174-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0174.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0174.out 0174-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0181
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0181.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0181.xml"
		0181-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0181.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0181.out 0181-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0197
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0197.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0197.xml"
		0197-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0197.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0197.out 0197-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0201
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0201.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0201.xml"
		0201-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0201.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0201.out 0201-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0202
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0202.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0202.xml"
		0202-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0202.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0202.out 0202-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0203
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0203.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0203.xml"
		0203-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0203.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0203.out 0203-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0207
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0207.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0207.xml"
		0207-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0207.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0207.out 0207-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0209
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0209.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0209.xml"
		0209-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0209.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0209.out 0209-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0210
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0210.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0210.xml"
		0210-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0210.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0210.out 0210-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0211
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0211.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0211.xml"
		0211-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0211.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0211.out 0211-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0212
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0212.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0212.xml"
		0212-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0212.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0212.out 0212-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0215
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0215.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0215.xml"
		0215-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0215.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0215.out 0215-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0258
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0258.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0258.xml"
		0258-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0258.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0258.out 0258-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0262
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0262.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0262.xml"
		0262-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0262.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0262.out 0262-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0291
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0291.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0291.xml"
		0291-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0291.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0291.out 0291-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0294
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0294.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0294.xml"
		0294-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0294.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0294.out 0294-res.nt"
	)

	RAPPER_RDFDIFF_TEST(rdfa.0304
		"${RAPPER} -f noNet -q -i rdfa10 -I http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0304.xml -o ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0304.xml"
		0304-res.nt
		"${RDFDIFF} -f ntriples -u http://rdfa.info/test-suite/test-cases/xhtml1/rdfa1.0/0304.xml -t ntriples ${CMAKE_CURRENT_SOURCE_DIR}/0304.out 0304-res.nt"
	)

ENDIF(RAPTOR_PARSER_RDFA)

# end raptor/tests/rdfa/CMakeLists.txt

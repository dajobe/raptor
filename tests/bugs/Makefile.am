TESTS=raptor_issue70a_test$(EXEEXT) raptor_issue70b_test$(EXEEXT)

AM_CPPFLAGS=-I$(top_srcdir)/src
AM_CFLAGS= -I$(top_builddir)/src @CFLAGS@ $(MEM)
AM_LDFLAGS=$(top_builddir)/src/libraptor2.la $(MEM_LIBS)

EXTRA_PROGRAMS=$(TESTS)

CLEANFILES=$(TESTS)

raptor_issue70a_test_SOURCES=issue70a.c
raptor_issue70b_test_SOURCES=issue70b.c


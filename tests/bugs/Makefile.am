TESTS=raptor_issue70a_test$(EXEEXT) raptor_issue70b_test$(EXEEXT)

AM_CPPFLAGS=-I$(top_srcdir)/src
AM_CFLAGS= -I$(top_builddir)/src @CFLAGS@ $(MEM)
AM_LDFLAGS=$(MEM_LIBS)

RAPTOR_IMPL_LIBS=$(top_builddir)/src/libraptor2_impl.la
if LIBRDFA
RAPTOR_IMPL_LIBS += $(top_builddir)/librdfa/librdfa.la
endif
RAPTOR_IMPL_LIBS += @RAPTOR_LDFLAGS@

EXTRA_PROGRAMS=$(TESTS)

CLEANFILES=$(TESTS)

raptor_issue70a_test_SOURCES=issue70a.c
raptor_issue70a_test_LDADD=$(RAPTOR_IMPL_LIBS)
raptor_issue70b_test_SOURCES=issue70b.c
raptor_issue70b_test_LDADD=$(RAPTOR_IMPL_LIBS)

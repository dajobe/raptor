# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor fuzz targets
#

if ENABLE_FUZZING

AM_CPPFLAGS = -I$(top_srcdir)/src -I$(top_builddir)/src -DRAPTOR_USE_LIBFUZZER=1
AM_CFLAGS = $(MEM) $(FUZZ_CFLAGS)
AM_LDFLAGS = $(FUZZ_LDFLAGS) -Wl,-rpath,$(abs_top_builddir)/src/.libs

noinst_PROGRAMS = fuzz_turtle

fuzz_turtle_SOURCES = fuzz_turtle.c
fuzz_turtle_LDADD = $(top_builddir)/src/libraptor2.la $(MEM_LIBS)

endif

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

FUZZ_WORKERS ?= 8

.PHONY: fuzz-turtle
fuzz-turtle: fuzz_turtle
	@mkdir -p $(abs_top_builddir)/tests/fuzz/artifacts \
	  $(abs_top_builddir)/tests/fuzz/corpus/turtle
	./fuzz_turtle \
	  -max_total_time=300 \
	  -timeout=10 \
	  -jobs=$(FUZZ_WORKERS) -workers=$(FUZZ_WORKERS) \
	  -rss_limit_mb=4096 \
	  -use_value_profile=1 \
	  -print_final_stats=1 \
	  -artifact_prefix=$(abs_top_builddir)/tests/fuzz/artifacts/ \
	  $(abs_top_builddir)/tests/fuzz/corpus/turtle \
	  $(abs_top_srcdir)/tests/turtle

endif

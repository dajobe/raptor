# -*- Mode: Makefile -*-
#
# Makefile for librdfa convienience library
#

ANALYZE = clang
ANALYZE_FLAGS = "--analyze"
# Based on COMPILE target
ANALYZE_COMMAND = $(ANALYZE) \
	$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	$(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS) \
	$(ANALYZE_FLAGS)

if LIBRDFA
noinst_LTLIBRARIES = librdfa.la
AM_CPPFLAGS = -DLIBRDFA_IN_RAPTOR -I$(srcdir) -I$(top_srcdir)/src

librdfa_la_SOURCES = \
context.c \
curie.c \
iri.c \
language.c \
lists.c \
namespace.c \
rdfa.c \
rdfa_utils.c \
subject.c \
triple.c \
rdfa.h \
rdfa_utils.h \
config.h

if NEED_STRTOK_R
librdfa_la_SOURCES += \
strtok_r.c \
strtok_r.h
endif


# Do not need:
#   mingw32_utils.c
#   mingw32_utils.h

else

EXTRA_DIST = \
context.c \
curie.c \
iri.c \
language.c \
lists.c \
namespace.c \
rdfa.c \
rdfa_utils.c \
strtok_r.c \
subject.c \
triple.c \
rdfa.h \
rdfa_utils.h \
strtok_r.h \
config.h

endif

CLEANFILES=*.plist

if MAINTAINER_MODE
# Run Clang static analyzer over sources.
analyze: $(SOURCES)
	@list='$(SOURCES)'; \
	result=0; \
	for file in $$list; do \
	  if echo $$file | grep '\.c$$' >/dev/null 2>&1; then \
	    $(RECHO) "Analyzing $$file"; \
	    $(ANALYZE_COMMAND) $(srcdir)/$$file; \
	    status=$$?; \
	    if test $$status != 0; then \
	      result=1; \
	    fi; \
	  fi; \
	done; \
	set -e; exit $$result
endif

# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor libraptor
#
# Copyright (C) 2000-2011, David Beckett http://www.dajobe.org/
# Copyright (C) 2000-2005, University of Bristol, UK http://www.bristol.ac.uk/
# 
# This package is Free Software and part of Redland http://librdf.org/
# 
# It is licensed under the following three licenses as alternatives:
#   1. GNU Lesser General Public License (LGPL) V2.1 or any newer version
#   2. GNU General Public License (GPL) V2 or any newer version
#   3. Apache License, V2.0 or any newer version
# 
# You may not use this file except in compliance with at least one of
# the above three licenses.
# 
# See LICENSE.html or LICENSE.txt at the top of this package for the
# complete terms and further detail along with the license texts for
# the licenses in COPYING.LIB, COPYING and LICENSE-2.0.txt respectively.
# 
# 


lib_LTLIBRARIES = libraptor2.la

pkginclude_HEADERS = raptor.h
nodist_pkginclude_HEADERS = raptor2.h

noinst_HEADERS = raptor_internal.h

TESTS=raptor_parse_test raptor_rfc2396_test raptor_uri_test \
raptor_namespace_test strcasecmp_test raptor_www_test \
raptor_sequence_test raptor_stringbuffer_test \
raptor_uri_win32_test raptor_iostream_test raptor_xml_writer_test \
raptor_turtle_writer_test raptor_avltree_test raptor_term_test \
raptor_permute_test raptor_snprintf_test raptor_sort_r_test
if RAPTOR_PARSER_RDFXML
TESTS += raptor_set_test raptor_xml_test
endif

CLEANFILES=$(TESTS) \
turtle_lexer_test turtle_parser_test \
*.plist \
git-version.h

MAINTAINERCLEANFILES=turtle_lexer.c turtle_lexer.h \
turtle_parser.c turtle_parser.h turtle_parser.output

# Memory debugging
MEM=@MEM@
MEM_LIBS=@MEM_LIBS@

AM_CPPFLAGS = $(MEM)

ANALYZE = clang
ANALYZE_FLAGS = "--analyze"
# Based on COMPILE target
ANALYZE_COMMAND = $(ANALYZE) \
	$(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	$(CPPFLAGS) $(AM_CFLAGS) $(CFLAGS) \
	$(ANALYZE_FLAGS)

BUILT_SOURCES = turtle_lexer.c turtle_lexer.h turtle_parser.c turtle_parser.h

libraptor2_la_SOURCES = raptor_parse.c raptor_serialize.c \
raptor_rfc2396.c raptor_uri.c raptor_log.c raptor_locator.c \
raptor_namespace.c raptor_qname.c \
raptor_option.c raptor_general.c raptor_unicode.c \
raptor_www.c \
raptor_statement.c \
raptor_term.c \
raptor_sequence.c raptor_stringbuffer.c raptor_iostream.c \
raptor_xml.c raptor_xml_writer.c raptor_set.c turtle_common.c \
raptor_turtle_writer.c raptor_avltree.c snprintf.c \
raptor_json_writer.c raptor_memstr.c raptor_concepts.c \
raptor_syntax_description.c \
raptor_sax2.c raptor_escaped.c \
raptor_ntriples.c \
sort_r.c sort_r.h ssort.h
if RAPTOR_XML_LIBXML
libraptor2_la_SOURCES += raptor_libxml.c
endif
if RAPTOR_PARSER_RDFXML
libraptor2_la_SOURCES += raptor_rdfxml.c
endif
if RAPTOR_PARSER_TURTLE
libraptor2_la_SOURCES += turtle_lexer.c turtle_lexer.h turtle_parser.c turtle_parser.h turtle_common.h
else
if RAPTOR_PARSER_TRIG
libraptor2_la_SOURCES += turtle_lexer.c turtle_lexer.h turtle_parser.c turtle_parser.h turtle_common.h
endif
endif
if RAPTOR_PARSER_NTRIPLES
libraptor2_la_SOURCES += ntriples_parse.c
else
if RAPTOR_PARSER_NQUADS
libraptor2_la_SOURCES += ntriples_parse.c
endif
endif
if RAPTOR_RSS_COMMON
libraptor2_la_SOURCES += raptor_rss_common.c raptor_rss.h
endif
if RAPTOR_PARSER_RSS
libraptor2_la_SOURCES += raptor_rss.c
endif
if RAPTOR_PARSER_GRDDL
libraptor2_la_SOURCES += raptor_grddl.c
endif
if RAPTOR_PARSER_GUESS
libraptor2_la_SOURCES += raptor_guess.c
endif
if RAPTOR_PARSER_RDFA
libraptor2_la_SOURCES += raptor_librdfa.c
endif
if RAPTOR_PARSER_JSON
libraptor2_la_SOURCES += raptor_json.c
endif
if RAPTOR_SERIALIZER_RDFXML
libraptor2_la_SOURCES += raptor_serialize_rdfxml.c
endif

if RAPTOR_SERIALIZER_NTRIPLES
libraptor2_la_SOURCES += raptor_serialize_ntriples.c
else
if RAPTOR_SERIALIZER_NQUADS
libraptor2_la_SOURCES += raptor_serialize_ntriples.c
endif
endif

#raptor_abbrev.c required by both turtle and xml-abbrev
if RAPTOR_SERIALIZER_RDFXML_ABBREV
libraptor2_la_SOURCES += raptor_abbrev.c
else
if RAPTOR_SERIALIZER_TURTLE
libraptor2_la_SOURCES += raptor_abbrev.c
endif
endif

if RAPTOR_SERIALIZER_RDFXML_ABBREV
libraptor2_la_SOURCES += raptor_serialize_rdfxmla.c
endif
if RAPTOR_SERIALIZER_TURTLE
libraptor2_la_SOURCES += raptor_serialize_turtle.c
endif
if RAPTOR_SERIALIZER_RSS_1_0
libraptor2_la_SOURCES += raptor_serialize_rss.c
endif
if RAPTOR_SERIALIZER_DOT
libraptor2_la_SOURCES += raptor_serialize_dot.c
endif
if RAPTOR_SERIALIZER_HTML
libraptor2_la_SOURCES += raptor_serialize_html.c
endif
if RAPTOR_SERIALIZER_JSON
libraptor2_la_SOURCES += raptor_serialize_json.c
endif
if STRCASECMP
libraptor2_la_SOURCES += strcasecmp.c
endif
if PARSEDATE
libraptor2_la_SOURCES += parsedate.c
BUILT_SOURCES += parsedate.c
endif


libraptor2_la_LIBADD = 

if LIBRDFA
AM_CPPFLAGS += -DLIBRDFA_IN_RAPTOR -I$(top_srcdir)/librdfa
libraptor2_la_LIBADD += $(top_builddir)/librdfa/librdfa.la
endif

libraptor2_la_LDFLAGS = -version-info @RAPTOR_LIBTOOL_VERSION@ \
@RAPTOR_LDFLAGS@ $(MEM_LIBS)
libraptor2_la_LIBADD += @LTLIBOBJS@


EXTRA_DIST=\
CMakeLists.txt \
raptor_config_cmake.h.in \
raptor_permute_test.c \
raptor_www_test.c \
raptor_nfc_test.c \
raptor_win32.c \
$(man_MANS) \
turtle_lexer.l turtle_parser.y \
parsedate.y


nodist_libraptor2_la_SOURCES =

if MAINTAINER_MODE
nodist_libraptor2_la_SOURCES += git-version.h

# raptor_parse.c is the first source file so this ensures
# git-version.h is created before it and before the actual file
# dependencies are calculated as a side-effect of compilation
raptor_parse.c: git-version.h

# Actually it needs turtle_parser.h but nevermind
turtle_lexer.c: $(srcdir)/turtle_lexer.l turtle_parser.c $(top_srcdir)/scripts/fix-flex.pl
	$(AM_V_GEN) \
	$(LEX) -o$@ $(srcdir)/turtle_lexer.l; \
	$(PERL) $(top_srcdir)/scripts/fix-flex.pl $@ > turtle_lexer.t || rm -f $@; \
	mv -f turtle_lexer.t $@

turtle_lexer.h: turtle_lexer.c ; @exit 0

turtle_parser.c: $(srcdir)/turtle_parser.y $(top_srcdir)/scripts/fix-bison.pl
	$(AM_V_GEN) \
	$(BISON) $(srcdir)/turtle_parser.y; \
	$(PERL) $(top_srcdir)/scripts/fix-bison.pl turtle_parser.tab.c > $@; \
	mv -f turtle_parser.tab.h turtle_parser.h; \
	rm -f turtle_parser.tab.c

turtle_parser.h: turtle_parser.c ; @exit 0

parsedate.c: $(srcdir)/parsedate.y $(top_srcdir)/scripts/fix-bison.pl
	$(AM_V_GEN) \
	$(BISON) $(srcdir)/parsedate.y; \
	$(PERL) $(top_srcdir)/scripts/fix-bison.pl parsedate.tab.c > $@; \
	mv -f parsedate.tab.h parsedate.h; \
	rm -f parsedate.tab.c

parsedate.h: parsedate.c ; @exit 0

endif

# Actually it needs turtle_parser.h but nevermind
turtle_lexer_test: $(srcdir)/turtle_lexer.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/turtle_lexer.c libraptor2.la $(LIBS)

turtle_parser_test: $(srcdir)/turtle_parser.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/turtle_parser.c libraptor2.la $(LIBS)

raptor_parse_test: $(srcdir)/raptor_parse.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_parse.c libraptor2.la $(LIBS)

raptor_rfc2396_test: $(srcdir)/raptor_rfc2396.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_rfc2396.c libraptor2.la $(LIBS)

raptor_uri_test: $(srcdir)/raptor_uri.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_uri.c libraptor2.la $(LIBS)

raptor_uri_win32_test: $(srcdir)/raptor_uri.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE -DWIN32_URI_TEST $(srcdir)/raptor_uri.c libraptor2.la $(LIBS)

raptor_namespace_test: $(srcdir)/raptor_namespace.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_namespace.c libraptor2.la $(LIBS)

strcasecmp_test: $(srcdir)/strcasecmp.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/strcasecmp.c libraptor2.la $(LIBS)

raptor_www_test: $(srcdir)/raptor_www_test.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_www_test.c libraptor2.la $(LIBS)

raptor_set_test: $(srcdir)/raptor_set.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_set.c libraptor2.la $(LIBS)

raptor_xml_test: $(srcdir)/raptor_xml.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_xml.c libraptor2.la $(LIBS)

raptor_sequence_test: $(srcdir)/raptor_sequence.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_sequence.c libraptor2.la $(LIBS)

raptor_stringbuffer_test: $(srcdir)/raptor_stringbuffer.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_stringbuffer.c libraptor2.la $(LIBS)

raptor_nfc_test: $(srcdir)/raptor_nfc_test.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_nfc_test.c libraptor2.la $(LIBS)

raptor_iostream_test: $(srcdir)/raptor_iostream.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_iostream.c libraptor2.la $(LIBS)

raptor_xml_writer_test: $(srcdir)/raptor_xml_writer.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_xml_writer.c libraptor2.la $(LIBS)

raptor_turtle_writer_test: $(srcdir)/raptor_turtle_writer.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_turtle_writer.c libraptor2.la $(LIBS)

raptor_avltree_test: $(srcdir)/raptor_avltree.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_avltree.c libraptor2.la $(LIBS)

raptor_term_test: $(srcdir)/raptor_term.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_term.c libraptor2.la $(LIBS)

raptor_permute_test: $(srcdir)/raptor_permute_test.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_permute_test.c libraptor2.la $(LIBS)

raptor_snprintf_test: $(srcdir)/snprintf.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/snprintf.c libraptor2.la $(LIBS)

raptor_sort_r_test: $(srcdir)/sort_r.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/sort_r.c libraptor2.la $(LIBS)

$(top_builddir)/librdfa/librdfa.la:
	cd $(top_builddir)/librdfa && $(MAKE) librdfa.la 

# Some people need a little help ;-)
test: check


if MAINTAINER_MODE
git-version.h: check-version

# Always run this in maintainer mode but do not always change git-version.h
.PHONY: check-version
check-version:
	@file="git-version.h" ; \
	if test -d ../.git; then \
	  git_version=`cd .. && git rev-parse HEAD`; \
	else \
	  git_version="unknown"; \
	fi; \
	$(RECHO) "GIT version $$git_version"; \
	tmp="$$file.tmp"; \
	$(RECHO) "#define GIT_VERSION \"$$git_version\"" > $$tmp; \
        if test -f $$file; then \
          if cmp $$file $$tmp >/dev/null 2>&1; then \
	    rm $$tmp; \
	  else \
	    mv $$tmp $$file; \
          fi; \
	else \
	  mv $$tmp $$file; \
	fi

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

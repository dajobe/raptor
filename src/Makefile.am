# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor libraptor
#
# Copyright (C) 2000-2010, David Beckett http://www.dajobe.org/
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

nodist_pkginclude_HEADERS = raptor.h

noinst_HEADERS = raptor_internal.h win32_raptor_config.h

TESTS=raptor_parse_test raptor_rfc2396_test raptor_uri_test \
raptor_namespace_test strcasecmp_test raptor_www_test \
raptor_sequence_test raptor_stringbuffer_test \
raptor_uri_win32_test raptor_iostream_test raptor_xml_writer_test \
raptor_turtle_writer_test raptor_avltree_test raptor_term_test
if RAPTOR_PARSER_RDFXML
TESTS += raptor_set_test raptor_xml_test
endif

CLEANFILES=$(TESTS) \
turtle_lexer_test turtle_parser_test \
rdfdiff

MAINTAINERCLEANFILES=turtle_lexer.c turtle_lexer.h \
turtle_parser.c turtle_parser.h turtle_parser.output

# Memory debugging
MEM=@MEM@
MEM_LIBS=@MEM_LIBS@

AM_CPPFLAGS = $(MEM)


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
raptor_syntax_description.c
if RAPTOR_PARSER_RDFXML
if RAPTOR_XML_LIBXML
libraptor2_la_SOURCES += raptor_libxml.c
endif
if RAPTOR_XML_EXPAT
libraptor2_la_SOURCES += raptor_expat.c
endif
libraptor2_la_SOURCES += raptor_rdfxml.c raptor_sax2.c
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
if RAPTOR_NFC_CHECK
libraptor2_la_SOURCES += raptor_nfc_data.c raptor_nfc.c raptor_nfc.h
endif
if STRCASECMP
libraptor2_la_SOURCES += strcasecmp.c
endif
if PARSEDATE
libraptor2_la_SOURCES += parsedate.c
endif


if LIBRDFA
libraptor2_la_SOURCES += \
$(top_srcdir)/librdfa/curie.c \
$(top_srcdir)/librdfa/iri.c \
$(top_srcdir)/librdfa/language.c\
$(top_srcdir)/librdfa/rdfa.c \
$(top_srcdir)/librdfa/rdfa_utils.c \
$(top_srcdir)/librdfa/subject.c \
$(top_srcdir)/librdfa/triple.c \
$(top_srcdir)/librdfa/rdfa.h \
$(top_srcdir)/librdfa/rdfa_utils.h 

AM_CPPFLAGS += -DLIBRDFA_IN_RAPTOR -I$(top_srcdir)/librdfa
endif

libraptor2_la_LDFLAGS = -version-info @RAPTOR_LIBTOOL_VERSION@ \
@RAPTOR_LDFLAGS@ $(MEM_LIBS)
libraptor2_la_LIBADD = @LTLIBOBJS@


EXTRA_DIST=\
raptor_www_test.c \
raptor_nfc_test.c \
raptor_win32.c \
$(man_MANS) \
turtle_lexer.l turtle_parser.y \
parsedate.y


LEX=@LEX@
YACC=@YACC@

if MAINTAINER_MODE
# Actually it needs turtle_parser.h but nevermind
turtle_lexer.c: $(srcdir)/turtle_lexer.l turtle_parser.c $(srcdir)/../scripts/fix-flex
	$(LEX) -o$@ $(srcdir)/turtle_lexer.l
	$(PERL) $(srcdir)/../scripts/fix-flex $@ > turtle_lexer.t
	mv turtle_lexer.t $@

turtle_parser.c: $(srcdir)/turtle_parser.y $(srcdir)/../scripts/fix-bison
	$(YACC) -b turtle_parser -p turtle_parser_ -d -v $(srcdir)/turtle_parser.y
	$(PERL) $(srcdir)/../scripts/fix-bison turtle_parser.tab.c > $@
	mv turtle_parser.tab.h turtle_parser.h
	rm -f turtle_parser.tab.c

endif

# Actually it needs turtle_parser.h but nevermind
turtle_lexer_test: $(srcdir)/turtle_lexer.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/turtle_lexer.c libraptor2.la $(LIBS)

turtle_parser_test: $(srcdir)/turtle_parser.c libraptor2.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/turtle_parser.c libraptor2.la $(LIBS)


parsedate.c: $(srcdir)/parsedate.y
	$(YACC) -b parsedate -p raptor_parsedate_ -d -v $(srcdir)/parsedate.y
	sed -e '/Suppress GCC warning that yyerrlab1/,/^\#endif/d' -e "s/parsedate.tab.c/$@/" parsedate.tab.c > $@
	mv parsedate.tab.h parsedate.h
	rm -f parsedate.tab.c



# Some people need a little help ;-)
test: check

raptor_parse_test: $(srcdir)/raptor_general.c libraptor2.la
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

# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor
#
# $Id$
#
# Copyright (C) 2000-2004, David Beckett http://purl.org/net/dajobe/
# Institute for Learning and Research Technology http://www.ilrt.bristol.ac.uk/
# University of Bristol, UK http://www.bristol.ac.uk/
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


bin_PROGRAMS = rapper
EXTRA_PROGRAMS = rdfdiff

bin_SCRIPTS = raptor-config
noinst_SCRIPTS = raptor-src-config

lib_LTLIBRARIES = libraptor.la

include_HEADERS = raptor.h

noinst_HEADERS = raptor_internal.h win32_raptor_config.h

man_MANS = rapper.1 libraptor.3 raptor-config.1

SUBDIRS= tests win32 examples

TESTS=raptor_parse_test raptor_rfc2396_test raptor_uri_test \
raptor_namespace_test strcasecmp_test raptor_www_test \
raptor_sequence_test raptor_stringbuffer_test \
raptor_uri_win32_test raptor_iostream_test raptor_xml_writer_test
if RAPTOR_PARSER_RDFXML
TESTS += raptor_set_test raptor_xml_test
endif

CLEANFILES=$(TESTS) turtle_lexer_test turtle_parser_test

MAINTAINERCLEANFILES=turtle_lexer.c turtle_lexer.h \
turtle_parser.c turtle_parser.h turtle_parser.output

# Memory debugging alternatives
MEM=@MEM@
MEM_LIBS=@MEM_LIBS@

# 1) None (use standard functions directly)
#MEM=
#MEM_LIBS=

# 2) Use dmalloc library
#MEM=-DRAPTOR_MEMORY_DEBUG_DMALLOC=1
#MEM_LIBS=-ldmalloc

AM_CFLAGS=@CFLAGS@ $(MEM)
STANDARD_CFLAGS=@STANDARD_CFLAGS@ $(MEM)
LIBS=@LIBS@ $(MEM_LIBS)


libraptor_la_SOURCES = raptor_parse.c raptor_serialize.c raptor_serialize_rdfxmla.c\
raptor_rfc2396.c raptor_uri.c raptor_locator.c \
raptor_namespace.c raptor_qname.c \
raptor_feature.c raptor_general.c raptor_utf8.c \
raptor_www.c \
raptor_identifier.c \
raptor_sequence.c raptor_stringbuffer.c raptor_iostream.c
if RAPTOR_PARSER_RDFXML
libraptor_la_SOURCES += raptor_rdfxml.c raptor_xml.c raptor_xml_writer.c \
raptor_sax2.c raptor_set.c
if RAPTOR_XML_LIBXML
libraptor_la_SOURCES += raptor_libxml.c
endif
if RAPTOR_XML_EXPAT
libraptor_la_SOURCES += raptor_expat.c
endif
endif
if RAPTOR_PARSER_TURTLE
libraptor_la_SOURCES += turtle_lexer.c turtle_lexer.h turtle_parser.c turtle_parser.h turtle_common.h
endif
if RAPTOR_PARSER_N3
libraptor_la_SOURCES += n3_lexer.c n3_lexer.h n3_parser.c n3_parser.h n3_common.h
endif
if RAPTOR_PARSER_NTRIPLES
libraptor_la_SOURCES += ntriples_parse.c
endif
if RAPTOR_PARSER_RSS
libraptor_la_SOURCES += raptor_rss.c
endif
if RAPTOR_NFC_CHECK
libraptor_la_SOURCES += raptor_nfc_data.c raptor_nfc.c raptor_nfc.h
endif
if STRCASECMP
libraptor_la_SOURCES += strcasecmp.c
endif

libraptor_la_LDFLAGS = -version-info @RAPTOR_LIBTOOL_VERSION@
libraptor_la_LIBADD = @LTLIBOBJS@


rapper_SOURCES = rdfdump.c
if GETOPT
rapper_SOURCES += getopt.c raptor_getopt.h
endif
rapper_LDADD=libraptor.la

rdfdiff_SOURCES = rdfdiff.c
if GETOPT
rdfdiff_SOURCES += getopt.c raptor_getopt.h
endif
rdfdiff_LDADD=libraptor.la


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = raptor.pc


EXTRA_DIST=ChangeLog ChangeLog.1 \
README      NEWS      LICENSE.txt \
README.html NEWS.html LICENSE.html INSTALL.html \
libraptor.html rapper.html RELEASE.html \
LICENSE-2.0.txt NOTICE \
raptor.rdf.in \
raptor-config.in raptor-src-config.in \
autogen.sh \
raptor.spec.in \
dc.rdf \
raptor_www_test.c \
raptor_nfc_test.c \
raptor_win32.c \
fix-groff-xhtml \
$(man_MANS) \
raptor.pc.in \
turtle_lexer.l turtle_parser.y \
fix-flex


LEX=@LEX@
YACC=@YACC@

# Create some text files from HTML sources
LYNX=lynx
HTML_TO_TEXT=TERM=vt100 $(LYNX) -dump -nolist

SUFFIXES = .html .txt

.html.txt:
	$(HTML_TO_TEXT) $< > $@

README: README.html
	$(HTML_TO_TEXT) $< > $@

NEWS: NEWS.html
	$(HTML_TO_TEXT) $< > $@


if MAINTAINER_MODE
libraptor.html: $(srcdir)/libraptor.3 $(srcdir)/fix-groff-xhtml
	-groff -man -Thtml -P-l $< | tidy -asxml -wrap 1000 2>/dev/null | perl $(srcdir)/fix-groff-xhtml $@

rapper.html: $(srcdir)/rapper.1 $(srcdir)/fix-groff-xhtml
	-groff -man -Thtml -P-l $< | tidy -asxml -wrap 1000 2>/dev/null | perl $(srcdir)/fix-groff-xhtml $@


# Actually it needs turtle_parser.h but nevermind
turtle_lexer.c: $(srcdir)/turtle_lexer.l turtle_parser.c $(srcdir)/fix-flex
	$(LEX) -o$@ $(srcdir)/turtle_lexer.l
	$(PERL) $(srcdir)/fix-flex $@ > turtle_lexer.t
	mv turtle_lexer.t $@
	$(PERL) $(srcdir)/fix-flex turtle_lexer.h > turtle_lexer.t
	mv turtle_lexer.t turtle_lexer.h

turtle_parser.c: $(srcdir)/turtle_parser.y
	$(YACC) -b turtle_parser -p turtle_parser_ -d -v $(srcdir)/turtle_parser.y
	sed -e '/Suppress GCC warning that yyerrlab1/,/^\#endif/d' -e "s/turtle_parser.tab.c/$@/" turtle_parser.tab.c > $@
	mv turtle_parser.tab.h turtle_parser.h
	rm -f turtle_parser.tab.c

# Actually it needs n3_parser.h but nevermind
n3_lexer.c: $(srcdir)/n3_lexer.l n3_parser.c $(srcdir)/fix-flex
	$(LEX) -o$@ $(srcdir)/n3_lexer.l
	$(PERL) $(srcdir)/fix-flex $@ > n3_lexer.t
	mv n3_lexer.t $@
	$(PERL) $(srcdir)/fix-flex n3_lexer.h > n3_lexer.t
	mv n3_lexer.t n3_lexer.h

n3_parser.c: $(srcdir)/n3_parser.y
	$(YACC) -b n3_parser -p n3_parser_ -d -v $(srcdir)/n3_parser.y
	sed -e '/Suppress GCC warning that yyerrlab1/,/^\#endif/d' -e "s/n3_parser.tab.c/$@/" n3_parser.tab.c > $@
	mv n3_parser.tab.h n3_parser.h
	rm -f n3_parser.tab.c

endif

# Actually it needs turtle_parser.h but nevermind
turtle_lexer_test: $(srcdir)/turtle_lexer.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/turtle_lexer.c libraptor.la $(LIBS)

turtle_parser_test: $(srcdir)/turtle_parser.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/turtle_parser.c libraptor.la $(LIBS)

check-local: rapper rdfdiff

# Some people need a little help ;-)
test: check

raptor_parse_test: $(srcdir)/raptor_general.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_parse.c libraptor.la $(LIBS)

raptor_rfc2396_test: $(srcdir)/raptor_rfc2396.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_rfc2396.c libraptor.la $(LIBS)

raptor_uri_test: $(srcdir)/raptor_uri.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_uri.c libraptor.la $(LIBS)

raptor_uri_win32_test: $(srcdir)/raptor_uri.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE -DWIN32_URI_TEST $(srcdir)/raptor_uri.c libraptor.la $(LIBS)

raptor_namespace_test: $(srcdir)/raptor_namespace.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_namespace.c libraptor.la $(LIBS)

strcasecmp_test: $(srcdir)/strcasecmp.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/strcasecmp.c libraptor.la $(LIBS)

raptor_www_test: $(srcdir)/raptor_www_test.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_www_test.c libraptor.la $(LIBS)

raptor_set_test: $(srcdir)/raptor_set.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_set.c libraptor.la $(LIBS)

raptor_xml_test: $(srcdir)/raptor_xml.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_xml.c libraptor.la $(LIBS)

raptor_sequence_test: $(srcdir)/raptor_sequence.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_sequence.c libraptor.la $(LIBS)

raptor_stringbuffer_test: $(srcdir)/raptor_stringbuffer.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_stringbuffer.c libraptor.la $(LIBS)

raptor_nfc_test: $(srcdir)/raptor_nfc_test.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_nfc_test.c libraptor.la $(LIBS)

raptor_iostream_test: $(srcdir)/raptor_iostream.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_iostream.c libraptor.la $(LIBS)

raptor_xml_writer_test: $(srcdir)/raptor_xml_writer.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_xml_writer.c libraptor.la $(LIBS)



deb: distcheck
	rm -rf $(distdir)
	tar xfz $(distdir).tar.gz
	cd $distdir && dpkg-buildpackage -rfakeroot

dist-hook: README NEWS
	@for file in README NEWS; do \
          if test -r $(srcdir)/$$file; then \
	    rm -f $(distdir)/$$file; \
	    cp -p $(srcdir)/$$file $(distdir)/$$file; \
	  fi; \
	done

# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor
#
# $Id$
#
# Copyright (C) 2000-2004 David Beckett - http://purl.org/net/dajobe/
# Institute for Learning and Research Technology - http://www.ilrt.bris.ac.uk/
# University of Bristol - http://www.bristol.ac.uk/
# 
# This package is Free Software or Open Source available under the
# following licenses (these are alternatives):
#   1. GNU Lesser General Public License (LGPL)
#   2. GNU General Public License (GPL)
#   3. Mozilla Public License (MPL)
# 
# See LICENSE.html or LICENSE.txt at the top of this package for the
# full license terms.
# 


bin_PROGRAMS = rapper
bin_SCRIPTS = raptor-config
noinst_SCRIPTS = raptor-src-config
lib_LTLIBRARIES = libraptor.la

include_HEADERS = raptor.h

noinst_HEADERS = raptor_internal.h win32_config.h

man_MANS = rapper.1 libraptor.3 raptor-config.1

SUBDIRS= tests win32 examples

TESTS=raptor_general_test raptor_uri_test raptor_namespace_test	\
strcasecmp_test raptor_www_test raptor_set_test raptor_xml_test	\
raptor_sequence_test raptor_stringbuffer_test

CLEANFILES=$(TESTS) turtle_lexer_test turtle_parser_test

MAINTAINERCLEANFILES=turtle_lexer.c turtle_lexer.h \
turtle_parser.tab.c turtle_parser.tab.h turtle_parser.output

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


libraptor_la_SOURCES = raptor_parse.c ntriples_parse.c \
raptor_uri.c raptor_expat.c raptor_libxml.c raptor_win32.c raptor_locator.c \
raptor_namespace.c raptor_qname.c raptor_general.c raptor_utf8.c \
raptor_www.c raptor_set.c raptor_xml.c raptor_sax2.c \
raptor_identifier.c raptor_xml_writer.c \
turtle_lexer.c turtle_lexer.h turtle_parser.tab.c turtle_parser.tab.h turtle_common.h \
raptor_sequence.c raptor_stringbuffer.c
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


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = raptor.pc


EXTRA_DIST=ChangeLog ChangeLog.1 \
README      NEWS      LICENSE.txt \
README.html NEWS.html LICENSE.html INSTALL.html \
MPL.html libraptor.html rapper.html RELEASE.html \
raptor.rdf.in \
raptor-config.in raptor-src-config.in \
autogen.sh \
raptor.spec.in \
dc.rdf \
raptor_www_test.c \
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
libraptor.html: $(srcdir)/libraptor.3 $(srcdir)/fix-groff-xhtml $@
	-groff -man -Thtml -P-l $< | tidy -asxml -wrap 1000 2>/dev/null | perl $(srcdir)/fix-groff-xhtml $@

rapper.html: $(srcdir)/rapper.1 $(srcdir)/fix-groff-xhtml $@
	-groff -man -Thtml -P-l $< | tidy -asxml -wrap 1000 2>/dev/null | perl $(srcdir)/fix-groff-xhtml $@


# Actually it needs turtle_parser.h but nevermind
turtle_lexer.c: $(srcdir)/turtle_lexer.l turtle_parser.tab.c $(srcdir)/fix-flex
	$(LEX) -o$@ $(srcdir)/turtle_lexer.l
	$(PERL) $(srcdir)/fix-flex $@ > turtle_lexer.t
	mv turtle_lexer.t $@

turtle_parser.tab.c: $(srcdir)/turtle_parser.y
	$(YACC) -b turtle_parser -p turtle_parser_ -d -v $(srcdir)/turtle_parser.y
	mv $@ turtle_parser.t
	sed -e '/Suppress GCC warning that yyerrlab1/,/^\#endif/d' turtle_parser.t > $@
	rm -f turtle_parser.t
endif

# Actually it needs turtle_parser.h but nevermind
turtle_lexer_test: $(srcdir)/turtle_lexer.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/turtle_lexer.c libraptor.la $(LIBS)

turtle_parser_test: $(srcdir)/turtle_parser.tab.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/turtle_parser.tab.c libraptor.la $(LIBS)

check-local: rapper

# Some people need a little help ;-)
test: check

raptor_general_test: $(srcdir)/raptor_general.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_general.c libraptor.la $(LIBS)

raptor_uri_test: $(srcdir)/raptor_uri.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_uri.c libraptor.la $(LIBS)

raptor_namespace_test: $(srcdir)/raptor_namespace.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_namespace.c libraptor.la $(LIBS)

strcasecmp_test: $(srcdir)/strcasecmp.c
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/strcasecmp.c

raptor_www_test: $(srcdir)/raptor_www_test.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_www_test.c libraptor.la $(LIBS)

raptor_set_test: $(srcdir)/raptor_set.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_set.c libraptor.la $(LIBS)

raptor_xml_test: $(srcdir)/raptor_xml.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_xml.c libraptor.la $(LIBS)

raptor_sequence_test: $(srcdir)/raptor_sequence.c
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_sequence.c libraptor.la $(LIBS)

raptor_stringbuffer_test: $(srcdir)/raptor_stringbuffer.c
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_stringbuffer.c $(LIBS)



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

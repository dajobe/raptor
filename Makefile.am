# -*- Mode: Makefile -*-
#
# Makefile.am - automake file for Raptor
#
# $Id$
#
# Copyright (C) 2000-2003 David Beckett - http://purl.org/net/dajobe/
# Institute for Learning and Research Technology - http://www.ilrt.org/
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

rapper_SOURCES = rdfdump.c raptor_getopt.h
rapper_LDADD = libraptor.la @RAPPER_EXTRA_OBJS@ @REDLAND_LIBS@
rapper_DEPENDENCIES = libraptor.la @RAPPER_EXTRA_OBJS@ @REDLAND_LIBS@
EXTRA_rapper_SOURCES = getopt.c

libraptor_la_SOURCES = raptor_parse.c ntriples_parse.c \
raptor_uri.c raptor_libxml.c raptor_win32.c raptor_locator.c \
raptor_namespace.c raptor_qname.c raptor_general.c raptor_utf8.c \
raptor_www.c raptor_set.c raptor_xml.c raptor_sax2.c \
raptor_identifier.c raptor_xml_writer.c \
n3_lexer.c n3_lexer.h n3_parser.tab.c n3_parser.tab.h n3_common.h \
raptor_sequence.c raptor_stringbuffer.c
libraptor_la_LDFLAGS = -version-info @RAPTOR_LIBTOOL_VERSION@
libraptor_la_LIBADD = @LTLIBOBJS@

EXTRA_DIST=ChangeLog \
README      NEWS      LICENSE.txt \
README.html NEWS.html LICENSE.html INSTALL.html \
MPL.html libraptor.html \
raptor-config.in raptor-src-config.in \
autogen.sh \
raptor.spec.in \
dc.rdf \
raptor_www_test.c \
fix-groff-xhtml \
$(man_MANS) \
raptor.pc.in \
n3_lexer.l n3_parser.y



SUBDIRS= tests win32 examples

TESTS=raptor_uri_test raptor_namespace_test strcasecmp_test \
raptor_www_test raptor_set_test raptor_xml_test \
raptor_sequence_test raptor_stringbuffer_test

CLEANFILES=$(TESTS) n3_lexer_test n3_parser_test

MAINTAINERCLEANFILES=n3_lexer.c n3_lexer.h \
n3_parser.tab.c n3_parser.tab.h n3_parser.output

# Use tar, whatever it is called (better be GNU tar though)
TAR=@TAR@

# Why is this not in the default makefile?
CC=@CC@

# Memory debugging alternatives
MEM=@MEM@
MEM_LIBS=@MEM_LIBS@

# 1) None (use standard functions directly)
#MEM=
#MEM_LIBS=

# 2) Use dmalloc library
#MEM=-DRAPTOR_MEMORY_DEBUG_DMALLOC=1
#MEM_LIBS=-ldmalloc

LEX=@LEX@
YACC=@YACC@

AM_CFLAGS=@CFLAGS@ @REDLAND_CFLAGS@ $(MEM)
STANDARD_CFLAGS=@STANDARD_CFLAGS@ $(MEM)
LIBS=@LIBS@ $(MEM_LIBS)

LOCAL_LIB_DIR=lib

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

@MAINT@libraptor.html: $(srcdir)/libraptor.3
@MAINT@	-groff -man -Thtml -P-l $< | tidy -asxml -wrap 1000 2>/dev/null | perl $(srcdir)/fix-groff-xhtml $@

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = raptor.pc

# Actually it needs n3_parser.h but nevermind
@MAINT@n3_lexer.c: $(srcdir)/n3_lexer.l n3_parser.tab.c
@MAINT@	$(LEX) -o$@ $(srcdir)/n3_lexer.l
@MAINT@	mv $@ n3_lexer.t
@MAINT@	sed -e 's/\(^[ \t]*free(.*ptr.*\)$$/if(ptr) \1/' n3_lexer.t > $@
@MAINT@	rm -f n3_lexer.t

@MAINT@n3_parser.tab.c: $(srcdir)/n3_parser.y
@MAINT@	$(YACC) -b n3_parser -p n3_parser_ -d -v $(srcdir)/n3_parser.y


# Actually it needs n3_parser.h but nevermind
n3_lexer_test: $(srcdir)/n3_lexer.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/n3_lexer.c libraptor.la $(LIBS) @REDLAND_LIBS@

n3_parser_test: $(srcdir)/n3_parser.tab.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/n3_parser.tab.c libraptor.la $(LIBS) @REDLAND_LIBS@


# Some people need a little help ;-)
test: check

raptor_uri_test: $(srcdir)/raptor_uri.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_uri.c libraptor.la $(LIBS)

raptor_namespace_test: $(srcdir)/raptor_namespace.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_namespace.c libraptor.la $(LIBS) @REDLAND_LIBS@

strcasecmp_test: $(srcdir)/strcasecmp.c
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/strcasecmp.c

raptor_www_test: $(srcdir)/raptor_www_test.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_www_test.c libraptor.la $(LIBS) @REDLAND_LIBS@

raptor_set_test: $(srcdir)/raptor_set.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_set.c libraptor.la $(LIBS) @REDLAND_LIBS@

raptor_xml_test: $(srcdir)/raptor_xml.c libraptor.la
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_xml.c libraptor.la $(LIBS) @REDLAND_LIBS@

raptor_sequence_test: $(srcdir)/raptor_sequence.c
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_sequence.c $(LIBS) @REDLAND_LIBS@

raptor_stringbuffer_test: $(srcdir)/raptor_stringbuffer.c
	$(LINK) $(DEFS) $(CPPFLAGS) -I$(srcdir) -I. -DSTANDALONE $(srcdir)/raptor_stringbuffer.c $(LIBS) @REDLAND_LIBS@



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

@SET_MAKE@

../librdf/librdf.la:
	cd ../librdf && $(MAKE) librdf.la

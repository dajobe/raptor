## Process this file with automake to produce Makefile.in

$(top_builddir)/rdfdump:
	@(cd .. ; $(MAKE) rdfdump)

RDF_TEST_FILES=ex-00.rdf ex-01.rdf ex-02.rdf ex-03.rdf ex-04.rdf ex-05.rdf \
ex-06.rdf ex-07.rdf ex-08.rdf ex-09.rdf ex-10.rdf ex-11.rdf ex-12.rdf \
ex-13.rdf ex-14.rdf ex-15.rdf ex-16.rdf ex-17.rdf ex-18.rdf

RDF_TEST_ANSWER_FILES=ex-13.nt

NT_TEST_FILES=test.nt


EXTRA_DIST = \
	$(RDF_TEST_FILES) \
	$(RDF_TEST_ANSWER_FILES) \
	$(NT_TEST_FILES)


all: test

test tests: check-local

check-local: $(top_builddir)/rdfdump
	@for test in $(RDF_TEST_FILES); do \
	  name=`basename $$test .rdf` ; \
	  echo Checking $$test; \
	  $(top_builddir)/rdfdump -q file:$(srcdir)/$$test | sed -e 's/^.*rdfdump: //' > $$name.res; \
	  if [ ! -f $(srcdir)/$$name.out ] ; then cp $$name.res $(srcdir)/$$name.out ; \
	  else if  [ ! -s $$name.res ] ; then echo "Fatal error, no $$name.res\n" ; \
	  else diff $(srcdir)/$$name.out $$name.res ; fi ; fi; \
	  rm -f $$name.res ; \
	done; \
	for test in $(NT_TEST_FILES); do \
	  name=`basename $$test .nt` ; \
	  echo Checking $$test; \
	  $(top_builddir)/rdfdump -q -n file:$(srcdir)/$$test | sed -e 's/^.*rdfdump: //'  > $$name.res; \
	  if [ ! -f $(srcdir)/$$name.out ] ; then cp $$name.res $(srcdir)/$$name.out ; \
	  else if  [ ! -s $$name.res ] ; then echo "Fatal error, no $$name.res\n" ; \
	  else diff $(srcdir)/$$name.out $$name.res ; fi ; fi; \
	  rm -f $$name.res ; \
	done

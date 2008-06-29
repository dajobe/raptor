TEST_RSS=test01.rss test02.rss test03.rss
TEST_ATOMS=$(TEST_RSS:.rss=.atom)
EXTRA_ATOMS=

ATOMS=$(TEST_ATOMS) $(EXTRA_ATOMS)

rapper_dir=../../utils

.SUFFIXES: .atom .rss .rng .rnc

all: atom.rng $(TEST_ATOMS)

clean:
	rm -f $(TEST_ATOMS)

.rnc.rng:
	trang $? $@

check: check-atoms

check-atoms: atom.rng $(ATOMS)
	@for atom in $(ATOMS); do \
	  echo "Checking $$atom"; \
	  jing atom.rng $$atom; \
	done

# xmllint --relaxng atom.rng --noout $$atom

test01.atom: test01.rss $(rapper_dir)/rapper
	$(rapper_dir)/rapper -q -i rdfxml -o atom -O http://example.org/whatsnew -f writeBaseURI=0 -f rssTriples=atom-triples -f atomEntryUri=http://example.org/2006/03/28/blog-item $< > $@

test02.atom: test02.rss $(rapper_dir)/rapper
	$(rapper_dir)/rapper -q -i rdfxml -o atom -O http://librdf.org/raptor/ -f writeBaseURI=0 -f rssTriples=atom-triples $< > $@

test03.atom: test03.rss $(rapper_dir)/rapper
	$(rapper_dir)/rapper -q -i rdfxml -o atom -O http://example.org -f writeBaseURI=0 -f rssTriples=atom-triples $< > $@

$(rapper_dir)/rapper:
	cd $(rapper_dir) && $(MAKE) rapper

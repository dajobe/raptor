These are the tests for the mKR (my Knowledge Representation) Language
that must be passed by conformant systems.  See
  http://mkrmke.org/parser/
for the full conformance information.

The format is a set of exact parser and serializer format tests.

Parser tests are a pair of files:
  xxx.mkr xxx.nt
which are the input mKR file and the expected output NTriples file.

Serializer tests are a pair of files:
  xxx.ttl xxx.mkr  or  xxx.rdf xxx.mkr
which are the input Turtle or RDF file and the expected output mKR file.
which are the input mKR file and the expected output NTriples file.

Roundtrip tests are a pair of files:
  xxx.mkr xxx.mkr.mkr
which are the input mKR file and the expected output mKR file.

Richard H. McCullough Jan/10/2015

Raptor RDF Syntax Library
=========================

[Dave Beckett](https://www.dajobe.org/)
---------------------------------------

Overview
--------

[Raptor](https://librdf.org/raptor/) is a free software / Open Source C library that provides a set of parsers and serializers that generate Resource Description Framework (RDF) triples by parsing syntaxes or serialize the triples into a syntax.
The supported parsing syntaxes are RDF/XML, N-Quads, N-Triples 1.0 and 1.1, TRiG, Turtle 2008 and 2013, RDFa 1.0 and 1.1, RSS tag soup including all versions of RSS, Atom 0.3 and Atom 1.0, GRDDL and microformats for HTML, XHTML and XML.
The serializing syntaxes are RDF/XML (regular, abbreviated, XMP), Turtle 2013, N-Quads, N-Triples 1.1, Atom 1.0, RSS 1.0, GraphViz DOT, HTML, JSON and mKR.

Raptor was designed to work closely with the [Redland RDF library](https://librdf.org/) (**R**DF **P**arser **T**oolkit f**o**r **R**edland) but is entirely separate.
It is a portable library that works across many POSIX systems (Unix, GNU/Linux, BSDs, OSX, cygwin, win32).

A summary of the changes can be found in the [NEWS](NEWS.html) file, detailed API changes in the [release notes](RELEASE.html) and file-by-file changes in the [ChangeLog](ChangeLog).

Details of upgrading from Raptor 1 as described in the [Upgrading document](UPGRADING.html).

*   Designed to integrate well with [Redland](https://librdf.org/)
*   Parses content on the web if [libcurl](https://curl.se/libcurl/), [libxml2](https://gitlab.gnome.org/GNOME/libxml2/-/wikis/home) or BSD libfetch is available.
*   Supports all RDF terms including datatyped and XML literals
*   Optional features including parsers and serialisers can be selected at configure time.
*   Language bindings to Perl, PHP, Python and Ruby when used via Redland
*   No memory leaks
*   Fast
*   Standalone [rapper](rapper.html) RDF parser utility program

Known bugs and issues are recorded in the [Redland issue tracker](https://bugs.librdf.org/) or at [GitHub issues for Raptor](https://github.com/dajobe/raptor/issues).

Parsers
-------

### RDF/XML Parser

A Parser for the standard [RDF/XML syntax](https://www.w3.org/TR/rdf-syntax-grammar/).

*   Fully handles the [RDF/XML syntax updates](https://www.w3.org/TR/rdf-syntax-grammar/) for [XML Base](https://www.w3.org/TR/xmlbase/), `xml:lang`, RDF datatyping and Collections.
*   Handles all RDF vocabularies such as [FOAF](http://www.foaf-project.org/), [RSS 1.0](http://www.purl.org/rss/1.0/), [Dublin Core](http://dublincore.org/), [OWL](https://www.w3.org/TR/owl-features/), [DOAP](http://usefulinc.com/doap)
*   Handles `rdf:resource` / `resource` attributes
*   Uses [libxml](https://gitlab.gnome.org/GNOME/libxml2/-/wikis/home) XML parser

### N-Quads Parser

A parser for the [RDF 1.1 N-Quads - A line-based syntax for an RDF datasets](https://www.w3.org/TR/2013/CR-n-quads-20131105/), W3C Candidate Recommendation, 05 November 2013.
This is an extension to N-Triples, providing an optional 4th context graph term at the end of the line when a triple is associated with a named graph.

### N-Triples Parser

A parser for the [RDF 1.1 N-Triples - A line-based syntax for an RDF graph](https://www.w3.org/TR/2013/CR-n-triples-20131105/), W3C Candidate Recommendation, 05 November 2013 (aka N-Triples 2013) based on the older [N-Triples](https://www.w3.org/TR/rdf-testcases/#ntriples).

### Turtle Parser

A parser for the [Turtle Terse RDF Triple Language](https://www.w3.org/TR/2013/CR-turtle-20130219/) W3C Candidate Recommendation, 19 February 2013 based on earlier work [Turtle Terse RDF Triple Language](https://www.dajobe.org/2004/01/turtle/) (2004)

### TRiG Parser

A parser for the [RDF 1.1 TriG RDF Dataset Language](https://www.w3.org/TR/2014/REC-trig-20140225/).

The parser does not support the entire 1.1 TRiG specification; the '{' ... '}' around a graph and the `GRAPH` keyword may not be omitted.

### RSS "tag soup" parser

A parser for the multiple XML RSS formats that use the elements such as channel, item, title, description in different ways.
Attempts to turn the input into [RSS 1.0](http://www.purl.org/rss/1.0/) RDF triples.
True [RSS 1.0](http://www.purl.org/rss/1.0/), as a full RDF vocabulary, is best parsed by the RDF/XML parser.
It also generates triples for RSS enclosures.

This parser also provides support for the Atom 1.0 syndication format defined in IETF [RFC 4287](http://www.ietf.org/rfc/rfc4287.txt) as well as the earlier Atom 0.3.

### GRDDL and microformats parser

A parser/processor for [Gleaning Resource Descriptions from Dialects of Languages (GRDDL)](https://www.w3.org/TR/2007/REC-grddl-20070911/) syntax, W3C Recommendation of 2007-09-11 which allows reading XHTML and XML as RDF triples by using profiles in the document that declare XSLT transforms from the XHTML or XML content into RDF/XML or other RDF syntax which can then be parsed. It uses either an XML or a lax HTML parser to allow HTML tag soup to be read.

The parser passes the all the GRDDL tests as of Raptor 1.4.16.

The parser also handles hCard and hReview using public XSL sheets.

### RDFa parser

A parser for [RDFa 1.0](https://www.w3.org/TR/2008/REC-rdfa-syntax-20081014/) (W3C Recommendation 14 October 2008) and [RDFa 1.1](https://www.w3.org/TR/2012/REC-rdfa-core-20120607/) (W3C Recommendation 07 June 2012) implemented via [librdfa](https://github.com/rdfa/librdfa) linked inside Raptor.
librdfa was, written primarily by Manu Sporny of Digital Bazaar and is licensed with the same license as Raptor.

As of Raptor 2.0.8 the RDFa parser passes all of the [RDFa 1.0 test suite](https://www.w3.org/2006/07/SWD/RDFa/testsuite/xhtml1-testcases/) except for 4 tests and all of the [RDFa 1.1 test suite](http://rdfa.info/dev/) except for 30 tests.

Serializers
-----------

### RDF/XML Serializer

A serializer to the standard [RDF/XML syntax](https://www.w3.org/TR/rdf-syntax-grammar/) as revised by the [W3C RDF Core working group](https://www.w3.org/2001/sw/RDFCore/) in 2004.
This writes a plain triple-based RDF/XML serialization with no optimisation or pretty-printing.

A second serializer is provided using several of the RDF/XML abbreviations to provide a more compact readable format, at the cost of some pre-processing.
This is suitable for small documents.

### N-Quads Serializer

A serializer for the [RDF 1.1 N-Quads -A line-based syntax for an RDF datasets](https://www.w3.org/TR/2013/CR-n-quads-20131105/), W3C Candidate Recommendation, 05 November 2013.
This is an extension to N-Triples, providing an optional 4th context graph term at the end of the line when a triple is associated with a named graph.

### N-Triples Serializer

A serializer for the [RDF 1.1 N-Triples - A line-based syntax for an RDF graph](https://www.w3.org/TR/2013/CR-n-triples-20131105/) (aka N-Triples 2013) based on the earlier [N-Triples](https://www.w3.org/TR/rdf-testcases/#ntriples) syntax as used by the [W3C RDF Core working group](https://www.w3.org/2001/sw/RDFCore/) for the [RDF Test Cases](https://www.w3.org/TR/rdf-testcases/).

### Atom 1.0 Serializer

A serializer to the Atom 1.0 syndication format defined in IETF [RFC 4287](http://www.ietf.org/rfc/rfc4287.txt).

### JSON Serializers

Two serializers for to write triples encoded in JSON:

1.  `json`: in a resource-centric abbreviated form like Turtle or RDF/XML-Abbreviated as defined by: [RDF 1.1 JSON Alternate Serialization (RDF/JSON)](https://www.w3.org/TR/2013/NOTE-rdf-json-20131107/), W3C Working Group Note, 07 November 2013
2.  `json-triples`: a triple-centric format based on the SPARQL results in JSON format.

JSON-LD is not supported - too complex to implement.

### GraphViz DOT Serializer

An serializer to the GraphViz [DOT format](http://www.graphviz.org/doc/info/lang.html) which aids visualising RDF graphs.

### RSS 1.0 Serializer

A serializer to the [RDF Site Summary (RSS) 1.0](http://purl.org/rss/1.0/spec) format.

### Turtle Serializer

A serializer for the [Turtle Terse RDF Triple Language](https://www.w3.org/TR/2013/CR-turtle-20130219/) W3C Candidate Recommendation, 19 February 2013

### XMP Serializer

An alpha quality serializer to the Adobe XMP profile of RDF/XML suitable for embedding inside an external document.

### mKR Serializer

A serializer for the [mKR (my Knowledge Representation) Language](http://contextknowledgesystems.org/CKS.html)

Documentation
-------------

The public API is described in the [libraptor.3](libraptor.html) UNIX manual page.
It is demonstrated in the [rapper](rapper.html) utility program which shows how to call the parser and write the triples in a serialization.
When Raptor is used inside [Redland](https://librdf.org/), the Redland documentation explains how to call the parser and contains several example programs.
There are also further examples in the example directory of the distribution.

To install Raptor see the [Installation document](INSTALL.html).

Sources
-------

The packaged sources are available from [http://download.librdf.org/source/](http://download.librdf.org/source/) (master site) The development GIT sources can also be [browsed at GitHub](https://github.com/dajobe/raptor) or checked out at git://github.com/dajobe/raptor.git

License
-------

This library is free software / open source software released under the LGPL (GPL) or Apache 2.0 licenses.
See [LICENSE.html](LICENSE.html) for full details.

Mailing Lists
-------------

The [Redland mailing lists](https://librdf.org/lists/) discusses the development and use of Raptor and Redland as well as future plans and announcement of releases.

* * *

Copyright (C) 2000-2023 [Dave Beckett](https://www.dajobe.org/)  
Copyright (C) 2000-2005 [University of Bristol](https://www.bristol.ac.uk/)

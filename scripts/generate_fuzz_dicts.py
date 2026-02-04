#!/usr/bin/env python3
"""Generate libFuzzer dictionaries from test corpora.

The script combines curated token lists with frequent IRIs and namespaces
extracted from the existing test suites, then writes dict files under
tests/fuzz/dict.

Run from the repo root:
  python3 scripts/generate_fuzz_dicts.py
  black scripts/generate_fuzz_dicts.py
"""

from __future__ import annotations

import argparse
import re
from collections import Counter
from pathlib import Path
from typing import Iterable, Sequence


TURTLE_PREFIX_TOKENS = [
    "@prefix",
    "@base",
    "@keyword",
    "a",
    "PREFIX",
    "BASE",
    "GRAPH",
    "[",
    "]",
    "(",
    ")",
    "{",
    "}",
    ".",
    ";",
    ",",
    "^^",
    "@en",
    "@en-US",
    "true",
    "false",
    "_:%",
    "_:b",
    "<http://",
    "<https://",
    "<urn:",
]

TURTLE_QNAME_TOKENS = [
    "rdf:",
    "rdfs:",
    "xsd:",
    "foaf:",
    "owl:",
]

TURTLE_TRAILING_TOKENS = [
    '"""',
    "'''",
    "\\u",
    "\\U",
    "\\n",
    "\\t",
    "\\r",
    '\\"',
]

TURTLE_SEED_URIS = [
    "http://example.org/",
    "http://www.w3.org/2013/TurtleTests/",
    "http://www.w3.org/1999/02/22-rdf-syntax-ns#",
    "http://www.w3.org/2000/01/rdf-schema#",
    "http://www.w3.org/2001/XMLSchema#",
    "http://xmlns.com/foaf/0.1/",
    "http://purl.org/rss/1.0/",
]

NTRIPLES_PREFIX_TOKENS = [
    "<http://",
    "<https://",
    "<urn:",
    "<file:",
]

NTRIPLES_TRAILING_TOKENS = [
    "_:%",
    "_:b",
    ".",
    "@en",
    "@en-US",
    "^^",
    "rdf:",
    "rdfs:",
    "xsd:",
    "\\u",
    "\\U",
    "\\n",
    "\\t",
    "\\r",
    '\\"',
]

RDFXML_STATIC_TOKENS = [
    "<?xml",
    "<rdf:RDF",
    "</rdf:RDF>",
    'xmlns:rdf="http://www.w3.org/1999/02/22-rdf-syntax-ns#"',
    'xmlns:rdfs="http://www.w3.org/2000/01/rdf-schema#"',
    'xmlns:xsd="http://www.w3.org/2001/XMLSchema#"',
    'xmlns:owl="http://www.w3.org/2002/07/owl#"',
    'xmlns:foaf="http://xmlns.com/foaf/0.1/"',
    "http://www.w3.org/1999/02/22-rdf-syntax-ns#",
    "http://www.w3.org/2000/01/rdf-schema#",
    "http://www.w3.org/2002/07/owl#",
    'xml:base="',
    'xml:lang="en"',
    'rdf:about="',
    'rdf:resource="',
    'rdf:ID="',
    'rdf:nodeID="',
    'rdf:datatype="',
    'rdf:parseType="',
    "rdf:type",
    "rdf:first",
    "rdf:rest",
    "rdf:nil",
    "<rdf:Description",
    "</rdf:Description>",
    "<rdf:Bag>",
    "</rdf:Bag>",
    "<rdf:Seq>",
    "</rdf:Seq>",
    "<rdf:Alt>",
    "</rdf:Alt>",
    "<rdf:li>",
    "</rdf:li>",
    "<owl:Class>",
    "</owl:Class>",
    "<owl:Restriction>",
    "</owl:Restriction>",
    "<owl:onProperty>",
    "</owl:onProperty>",
    "<owl:hasValue>",
    "</owl:hasValue>",
    "<rdfs:label>",
    "</rdfs:label>",
    "<rdfs:comment>",
    "</rdfs:comment>",
    "<rdfs:subClassOf>",
    "</rdfs:subClassOf>",
    "<![CDATA[",
    "-->",
    "<!--",
    "?>",
]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate fuzz dictionaries")
    parser.add_argument(
        "--root",
        type=Path,
        default=Path(__file__).resolve().parents[1],
        help="Repository root (default: repo root)",
    )
    parser.add_argument(
        "--output",
        type=Path,
        help="Output directory for dict files (default: tests/fuzz/dict)",
    )
    return parser.parse_args()


def iter_files(paths: Sequence[Path], suffixes: Sequence[str]) -> Iterable[Path]:
    for base in paths:
        if not base.exists():
            continue
        for suffix in suffixes:
            yield from base.rglob(f"*{suffix}")


def read_text(path: Path) -> str:
    return path.read_text(errors="ignore")


def top_uris(counter: Counter[str], limit: int) -> list[str]:
    items = sorted(counter.items(), key=lambda item: (-item[1], item[0]))
    return [uri for uri, _count in items[:limit]]


def unique(entries: Iterable[str]) -> list[str]:
    seen: set[str] = set()
    ordered: list[str] = []
    for entry in entries:
        if entry in seen:
            continue
        seen.add(entry)
        ordered.append(entry)
    return ordered


def encode_token(token: str) -> str:
    escaped = (
        token.replace("\\", "\\\\")
        .replace("\n", "\\n")
        .replace("\r", "\\r")
        .replace("\t", "\\t")
        .replace('"', '\\"')
    )
    return f'"{escaped}"'


def extract_turtle_uris(root: Path) -> list[str]:
    prefix_re = re.compile(r"@prefix\s+[^:]*:\s*<([^>]+)>")
    base_re = re.compile(r"@base\s*<([^>]+)>")
    counter: Counter[str] = Counter()
    for path in iter_files(
        [root / "tests/turtle", root / "tests/turtle-2013"], [".ttl"]
    ):
        text = read_text(path)
        for match in prefix_re.finditer(text):
            counter[match.group(1)] += 1
        for match in base_re.finditer(text):
            counter[match.group(1)] += 1
    top = []
    for uri in top_uris(counter, 12):
        if uri.startswith(("http://", "https://", "urn:", "file:")):
            top.append(uri)
    return top


def extract_ntriples_uris(root: Path) -> list[str]:
    iri_re = re.compile(r"<([^>]+)>")
    counter: Counter[str] = Counter()
    for path in iter_files(
        [root / "tests/ntriples", root / "tests/ntriples-2013"], [".nt", ".nq"]
    ):
        text = read_text(path)
        for match in iri_re.finditer(text):
            counter[match.group(1)] += 1
    top = []
    for uri in top_uris(counter, 20):
        if uri.startswith(("http://", "https://", "urn:", "file:")):
            top.append(uri)
    return top


def extract_rdfxml_namespaces(root: Path) -> list[str]:
    xmlns_re = re.compile(r"xmlns:[^=]+=\"([^\"]+)\"")
    counter: Counter[str] = Counter()
    for path in iter_files([root / "tests/rdfxml"], [".rdf", ".xml"]):
        text = read_text(path)
        for match in xmlns_re.finditer(text):
            counter[match.group(1)] += 1
    return top_uris(counter, 10)


def build_turtle_entries(root: Path) -> list[str]:
    uris = unique(TURTLE_SEED_URIS + extract_turtle_uris(root))
    uri_tokens = [f"<{uri}" for uri in uris]
    return unique(
        TURTLE_PREFIX_TOKENS
        + uri_tokens
        + TURTLE_QNAME_TOKENS
        + TURTLE_TRAILING_TOKENS
    )


def build_ntriples_entries(root: Path) -> list[str]:
    uris = extract_ntriples_uris(root)
    uri_tokens = [f"<{uri}>" for uri in uris]
    return unique(NTRIPLES_PREFIX_TOKENS + uri_tokens + NTRIPLES_TRAILING_TOKENS)


def build_rdfxml_entries(root: Path) -> list[str]:
    namespaces = extract_rdfxml_namespaces(root)
    return unique(RDFXML_STATIC_TOKENS + namespaces)


def write_dict(path: Path, entries: Sequence[str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    payload = "\n".join(encode_token(entry) for entry in entries) + "\n"
    path.write_text(payload)


def main() -> int:
    args = parse_args()
    root = args.root
    output = args.output or root / "tests/fuzz/dict"
    write_dict(output / "turtle.dict", build_turtle_entries(root))
    write_dict(output / "ntriples.dict", build_ntriples_entries(root))
    write_dict(output / "rdfxml.dict", build_rdfxml_entries(root))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
